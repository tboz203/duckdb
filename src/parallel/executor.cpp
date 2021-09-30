#include "duckdb/execution/executor.hpp"

#include "duckdb/execution/operator/helper/physical_execute.hpp"
#include "duckdb/execution/operator/join/physical_delim_join.hpp"
#include "duckdb/execution/operator/scan/physical_chunk_scan.hpp"
#include "duckdb/execution/operator/set/physical_recursive_cte.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/execution/execution_context.hpp"
#include "duckdb/parallel/thread_context.hpp"
#include "duckdb/parallel/task_scheduler.hpp"
#include "duckdb/parallel/pipeline_executor.hpp"

#include "duckdb/parallel/pipeline_event.hpp"
#include "duckdb/parallel/pipeline_finish_event.hpp"
#include "duckdb/parallel/pipeline_complete_event.hpp"

#include <algorithm>

namespace duckdb {

Executor::Executor(ClientContext &context) : context(context) {
}

Executor::~Executor() {
}

void Executor::AddEvent(shared_ptr<Event> event) {
	lock_guard<mutex> elock(executor_lock);
	events.push_back(move(event));
}

struct PipelineEventStack {
	Event *pipeline_event;
	Event *pipeline_finish_event;
	Event *pipeline_complete_event;
};

void Executor::ScheduleUnionPipeline(const shared_ptr<Pipeline> &pipeline, PipelineEventStack &parent_stack, unordered_map<Pipeline *, PipelineEventStack> &event_map) {
	pipeline->Ready();

	D_ASSERT(pipeline);
	auto pipeline_event = make_shared<PipelineEvent>(pipeline);

	PipelineEventStack stack;
	stack.pipeline_event = pipeline_event.get();
	stack.pipeline_finish_event = nullptr;
	stack.pipeline_complete_event = nullptr;

	parent_stack.pipeline_finish_event->AddDependency(*pipeline_event);

	events.push_back(move(pipeline_event));

	auto union_entry = union_pipelines.find(pipeline.get());
	if (union_entry != union_pipelines.end()) {
		for(auto &entry : union_entry->second) {
			ScheduleUnionPipeline(entry, parent_stack, event_map);
		}
	}
	event_map.insert(make_pair(pipeline.get(), stack));
}

void Executor::ScheduleChildPipeline(const shared_ptr<Pipeline> &pipeline, unordered_map<Pipeline *, PipelineEventStack> &event_map) {
	pipeline->Ready();

	auto child_ptr = pipeline.get();
	auto dependencies = child_dependencies.find(child_ptr);
	D_ASSERT(union_pipelines.find(child_ptr) == union_pipelines.end());
	D_ASSERT(dependencies != child_dependencies.end());
	// create the pipeline event and the event stack
	auto pipeline_event = make_shared<PipelineEvent>(pipeline);

	PipelineEventStack stack;
	stack.pipeline_event = pipeline_event.get();
	stack.pipeline_finish_event = nullptr;
	stack.pipeline_complete_event = nullptr;

	// set up the dependencies for this child pipeline
	for(auto &dep : dependencies->second) {
		auto dep_entry = event_map.find(dep);
		D_ASSERT(dep_entry != event_map.end());
		D_ASSERT(dep_entry->second.pipeline_event);
		stack.pipeline_event->AddDependency(*dep_entry->second.pipeline_event);
		if (dep_entry->second.pipeline_finish_event) {
			dep_entry->second.pipeline_finish_event->AddDependency(*stack.pipeline_event);
		}
	}

	events.push_back(move(pipeline_event));

	event_map.insert(make_pair(child_ptr, stack));
}

void Executor::SchedulePipeline(const shared_ptr<Pipeline> &pipeline, unordered_map<Pipeline *, PipelineEventStack> &event_map) {
	D_ASSERT(pipeline);

	pipeline->Ready();

	auto pipeline_event = make_shared<PipelineEvent>(pipeline);
	auto pipeline_finish_event = make_shared<PipelineFinishEvent>(pipeline);
	auto pipeline_complete_event = make_shared<PipelineCompleteEvent>(pipeline->executor);

	PipelineEventStack stack;
	stack.pipeline_event = pipeline_event.get();
	stack.pipeline_finish_event = pipeline_finish_event.get();
	stack.pipeline_complete_event = pipeline_complete_event.get();

	pipeline_finish_event->AddDependency(*pipeline_event);
	pipeline_complete_event->AddDependency(*pipeline_finish_event);

	events.push_back(move(pipeline_event));
	events.push_back(move(pipeline_finish_event));
	events.push_back(move(pipeline_complete_event));

	auto union_entry = union_pipelines.find(pipeline.get());
	if (union_entry != union_pipelines.end()) {
		for(auto &entry : union_entry->second) {
			ScheduleUnionPipeline(entry, stack, event_map);
		}
	}
	event_map.insert(make_pair(pipeline.get(), stack));
}

void Executor::ScheduleEvents() {
	D_ASSERT(events.empty());
	// create all the required pipeline events
	unordered_map<Pipeline *, PipelineEventStack> event_map;
	for(auto &pipeline : pipelines) {
		SchedulePipeline(pipeline, event_map);
	}
	// schedule child pipelines
	for(auto &entry : child_pipelines) {
		for(auto &child_entry : entry.second) {
			ScheduleChildPipeline(child_entry, event_map);
		}
	}
	// set up the dependencies between pipeline events
	for(auto &entry : event_map) {
		auto pipeline = entry.first;
		for(auto &dependency : pipeline->dependencies) {
			auto dep = dependency.lock();
			D_ASSERT(dep);
			auto &dep_entry = event_map[dep.get()];
			D_ASSERT(dep_entry.pipeline_complete_event);
			entry.second.pipeline_event->AddDependency(*dep_entry.pipeline_complete_event);
		}
	}
	// schedule the pipelines that do not have dependencies
	for(auto &event : events) {
		if (!event->HasDependencies()) {
			event->Schedule();
		}
	}
}

void Executor::ExtractPipelines(shared_ptr<Pipeline> &pipeline, vector<shared_ptr<Pipeline>> &result) {
	pipeline->Ready();

	auto pipeline_ptr = pipeline.get();
	result.push_back(move(pipeline));
	auto union_entry = union_pipelines.find(pipeline_ptr);
	if (union_entry != union_pipelines.end()) {
		for(auto &entry : union_entry->second) {
			ExtractPipelines(entry, result);
		}
		union_pipelines.erase(pipeline_ptr);
	}
	auto child_entry = child_pipelines.find(pipeline_ptr);
	if (child_entry != child_pipelines.end()) {
		for(auto &entry : child_entry->second) {
			ExtractPipelines(entry, result);
		}
		child_pipelines.erase(pipeline_ptr);
	}
}

bool Executor::NextExecutor() {
	if (root_pipeline_idx >= root_pipelines.size()) {
		return false;
	}
	root_executor = make_unique<PipelineExecutor>(context, *root_pipelines[root_pipeline_idx]);
	root_pipeline_idx++;
	return true;
}

void Executor::VerifyPipeline(Pipeline &pipeline) {
	D_ASSERT(!pipeline.ToString().empty());
	auto operators = pipeline.GetOperators();
	for (auto &other_pipeline : pipelines) {
		auto other_operators = other_pipeline->GetOperators();
		for(idx_t op_idx = 0; op_idx < operators.size(); op_idx++) {
			for(idx_t other_idx = 0; other_idx < other_operators.size(); other_idx++) {
				auto &left = *operators[op_idx];
				auto &right = *other_operators[other_idx];
				if (left.Equals(right)) {
					D_ASSERT(right.Equals(left));
				} else {
					D_ASSERT(!right.Equals(left));
				}
			}
		}
	}
}

void Executor::VerifyPipelines() {
#ifdef DEBUG
		for (auto &pipeline : pipelines) {
			VerifyPipeline(*pipeline);
		}
		for (auto &pipeline : root_pipelines) {
			VerifyPipeline(*pipeline);
		}
#endif
}

void Executor::Initialize(PhysicalOperator *plan) {
	Reset();

	auto &scheduler = TaskScheduler::GetScheduler(context);
	{
		lock_guard<mutex> elock(executor_lock);
		physical_plan = plan;

		context.profiler->Initialize(physical_plan);
		this->producer = scheduler.CreateProducer();

		auto root_pipeline = make_shared<Pipeline>(*this);
		root_pipeline->sink = nullptr;
		BuildPipelines(physical_plan, root_pipeline.get());

		this->total_pipelines = pipelines.size();

		root_pipeline_idx = 0;
		ExtractPipelines(root_pipeline, root_pipelines);

		VerifyPipelines();

		ScheduleEvents();
	}

	// now execute tasks from this producer until all pipelines are completed
	while (completed_pipelines < total_pipelines) {
		unique_ptr<Task> task;
		while (scheduler.GetTaskFromProducer(*producer, task)) {
			task->Execute();
			task.reset();
		}
		string exception;
		if (!GetError(exception)) {
			// no exceptions: continue
			continue;
		}

		// an exception has occurred executing one of the pipelines
		// we need to wait until all threads are finished
		// we do this by creating weak pointers to all pipelines
		// then clearing our references to the pipelines
		// and waiting until all pipelines have been destroyed
		vector<weak_ptr<Pipeline>> weak_references;
		{
			lock_guard<mutex> elock(executor_lock);
			weak_references.reserve(pipelines.size());
			for (auto &pipeline : pipelines) {
				weak_references.push_back(weak_ptr<Pipeline>(pipeline));
			}
			pipelines.clear();
			events.clear();
		}
		for (auto &weak_ref : weak_references) {
			while (true) {
				auto weak = weak_ref.lock();
				if (!weak) {
					break;
				}
			}
		}
		throw Exception(exception);
	}

	lock_guard<mutex> elock(executor_lock);
	pipelines.clear();
	NextExecutor();
	if (!exceptions.empty()) { // LCOV_EXCL_START
		// an exception has occurred executing one of the pipelines
		throw Exception(exceptions[0]);
	} // LCOV_EXCL_STOP
}

void Executor::Reset() {
	lock_guard<mutex> elock(executor_lock);
	delim_join_dependencies.clear();
	recursive_cte = nullptr;
	physical_plan = nullptr;
	root_executor.reset();
	root_pipelines.clear();
	root_pipeline_idx = 0;
	completed_pipelines = 0;
	total_pipelines = 0;
	exceptions.clear();
	pipelines.clear();
	events.clear();
	union_pipelines.clear();
	child_pipelines.clear();
	child_dependencies.clear();
}

void Executor::AddChildPipeline(Pipeline *current) {
	D_ASSERT(!current->operators.empty());
	// found another operator that is a source
	// schedule a child pipeline
	auto child_pipeline = make_shared<Pipeline>(*this);
	child_pipeline->sink = current->sink;
	child_pipeline->operators = current->operators;
	child_pipeline->source = current->operators.back();
	D_ASSERT(child_pipeline->source->IsSource());
	child_pipeline->operators.pop_back();

	vector<Pipeline *> dependencies;
	dependencies.push_back(current);
	auto entry = union_pipelines.find(current);
	if (entry != union_pipelines.end()) {
		for(auto &union_pipeline : entry->second) {
			dependencies.push_back(union_pipeline.get());
		}
	}
	D_ASSERT(child_dependencies.find(child_pipeline.get()) == child_dependencies.end());
	child_dependencies.insert(make_pair(child_pipeline.get(), move(dependencies)));
	child_pipelines[current].push_back(move(child_pipeline));
}

void Executor::BuildPipelines(PhysicalOperator *op, Pipeline *current) {
	D_ASSERT(current);
	if (op->IsSink()) {
		// operator is a sink, build a pipeline
		auto pipeline = make_shared<Pipeline>(*this);
		pipeline->sink = op;
		op->sink_state.reset();

		// the current is dependent on this pipeline to complete
		current->AddDependency(pipeline);
		PhysicalOperator *pipeline_child = nullptr;
		switch (op->type) {
		case PhysicalOperatorType::CREATE_TABLE_AS:
		case PhysicalOperatorType::INSERT:
		case PhysicalOperatorType::DELETE_OPERATOR:
		case PhysicalOperatorType::UPDATE:
		case PhysicalOperatorType::HASH_GROUP_BY:
		case PhysicalOperatorType::SIMPLE_AGGREGATE:
		case PhysicalOperatorType::PERFECT_HASH_GROUP_BY:
		case PhysicalOperatorType::WINDOW:
		case PhysicalOperatorType::ORDER_BY:
		case PhysicalOperatorType::RESERVOIR_SAMPLE:
		case PhysicalOperatorType::TOP_N:
		case PhysicalOperatorType::COPY_TO_FILE:
		case PhysicalOperatorType::EXPRESSION_SCAN:
		case PhysicalOperatorType::LIMIT:
			D_ASSERT(op->children.size() == 1);
			// single operator:
			// the operator becomes the data source of the current pipeline
			current->source = op;
			// we create a new pipeline starting from the child
			pipeline_child = op->children[0].get();
		case PhysicalOperatorType::EXPORT:
			// EXPORT has an optional child
			// we only need to schedule child pipelines if there is a child
			current->source = op;
			if (op->children.empty()) {
				return;
			}
			D_ASSERT(op->children.size() == 1);
			pipeline_child = op->children[0].get();
			break;
		case PhysicalOperatorType::NESTED_LOOP_JOIN:
		case PhysicalOperatorType::BLOCKWISE_NL_JOIN:
		case PhysicalOperatorType::HASH_JOIN:
		case PhysicalOperatorType::PIECEWISE_MERGE_JOIN:
		case PhysicalOperatorType::CROSS_PRODUCT:
			// regular join, create a pipeline with RHS source that sinks into this pipeline
			pipeline_child = op->children[1].get();
			// on the LHS (probe child), the operator becomes a regular operator
			current->operators.push_back(op);
			if (op->IsSource()) {
				AddChildPipeline(current);
			}
			BuildPipelines(op->children[0].get(), current);
			break;
		case PhysicalOperatorType::DELIM_JOIN: {
			// duplicate eliminated join
			// for delim joins, recurse into the actual join
			pipeline_child = op->children[0].get();
			break;
		}
		default:
			throw InternalException("Unimplemented sink type!");
		}
		D_ASSERT(pipeline_child);
		// recurse into the pipeline child
		BuildPipelines(pipeline_child, pipeline.get());
		if (op->type == PhysicalOperatorType::DELIM_JOIN) {
			// for delim joins, recurse into the actual join
			// any pipelines in there depend on the main pipeline
			auto &delim_join = (PhysicalDelimJoin &)*op;
			// any scan of the duplicate eliminated data on the RHS depends on this pipeline
			// we add an entry to the mapping of (PhysicalOperator*) -> (Pipeline*)
			for (auto &delim_scan : delim_join.delim_scans) {
				delim_join_dependencies[delim_scan] = pipeline.get();
			}
			BuildPipelines(delim_join.join.get(), current);
		}
		// regular pipeline: schedule it
		pipelines.push_back(move(pipeline));
	} else {
		// operator is not a sink! recurse in children
		// first check if there is any additional action we need to do depending on the type
		switch (op->type) {
		case PhysicalOperatorType::DELIM_SCAN: {
			D_ASSERT(op->children.empty());
			auto entry = delim_join_dependencies.find(op);
			D_ASSERT(entry != delim_join_dependencies.end());
			// this chunk scan introduces a dependency to the current pipeline
			// namely a dependency on the duplicate elimination pipeline to finish
			auto delim_dependency = entry->second->shared_from_this();
			D_ASSERT(delim_dependency->sink->type == PhysicalOperatorType::DELIM_JOIN);
			auto &delim_join = (PhysicalDelimJoin &)*delim_dependency->sink;
			current->AddDependency(delim_dependency);
			current->source = (PhysicalOperator *) delim_join.distinct.get();
			return;
		}
		case PhysicalOperatorType::EXECUTE: {
			// EXECUTE statement: build pipeline on child
			auto &execute = (PhysicalExecute &)*op;
			BuildPipelines(execute.plan, current);
			return;
		}
		case PhysicalOperatorType::RECURSIVE_CTE: {
			throw InternalException("FIXME: recursive CTEs");
		}
		case PhysicalOperatorType::RECURSIVE_CTE_SCAN: {
			throw InternalException("FIXME: recursive CTEs");
		}
		case PhysicalOperatorType::INDEX_JOIN: {
			// index join: we only continue into the LHS
			// the right side is probed by the index join
			// so we don't need to do anything in the pipeline with this child
			current->operators.push_back(op);
			BuildPipelines(op->children[0].get(), current);
			return;
		}
		case PhysicalOperatorType::UNION: {
			auto union_pipeline = make_shared<Pipeline>(*this);
			auto pipeline_ptr = union_pipeline.get();
			union_pipeline->operators = current->operators;
			union_pipeline->dependencies = current->dependencies;
			BuildPipelines(op->children[0].get(), current);
			union_pipelines[current].push_back(move(union_pipeline));

			pipeline_ptr->sink = current->sink;
			BuildPipelines(op->children[1].get(), pipeline_ptr);
			return;
		}
		default:
			break;
		}
		if (op->children.empty()) {
			// source
			current->source = op;
		} else {
			if (op->children.size() != 1) {
				throw InternalException("Operator not supported yet");
			}
			current->operators.push_back(op);
			BuildPipelines(op->children[0].get(), current);
		}
	}
}

vector<LogicalType> Executor::GetTypes() {
	D_ASSERT(physical_plan);
	return physical_plan->GetTypes();
}

void Executor::PushError(const string &exception) {
	lock_guard<mutex> elock(executor_lock);
	// interrupt execution of any other pipelines that belong to this executor
	context.interrupted = true;
	// push the exception onto the stack
	exceptions.push_back(exception);
}

bool Executor::GetError(string &exception) {
	lock_guard<mutex> elock(executor_lock);
	if (exceptions.empty()) {
		return false;
	}
	exception = exceptions[0];
	return true;
}

void Executor::Flush(ThreadContext &tcontext) {
	lock_guard<mutex> elock(executor_lock);
	context.profiler->Flush(tcontext.profiler);
}

bool Executor::GetPipelinesProgress(int &current_progress) { // LCOV_EXCL_START
	lock_guard<mutex> elock(executor_lock);

	if (!pipelines.empty()) {
		return pipelines.back()->GetProgress(current_progress);
	} else {
		current_progress = -1;
		return true;
	}
} // LCOV_EXCL_STOP

unique_ptr<DataChunk> Executor::FetchChunk() {
	D_ASSERT(physical_plan);

	auto chunk = make_unique<DataChunk>();
	root_executor->InitializeChunk(*chunk);
	while(true) {
		root_executor->ExecutePull(*chunk);
		if (chunk->size() == 0) {
			root_executor->PullFinalize();
			if (NextExecutor()) {
				continue;
			}
			break;
		} else {
			break;
		}
	}
	return chunk;
}

} // namespace duckdb
