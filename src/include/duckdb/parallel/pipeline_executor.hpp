//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parallel/pipeline_executor.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/parallel/pipeline.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/parallel/thread_context.hpp"
#include "duckdb/execution/execution_context.hpp"
#include "duckdb/common/stack.hpp"

#include <functional>

namespace duckdb {
class Executor;

//! The result of a pipeline execute call
enum class PipelineExecuteResult {
	//! Pipeline is fully executed: the source is completely exhausted
	FINISHED,
	//! Pipeline is not yet fully executed and can be called again immediately
	NOT_FINISHED,
	//! The pipeline was interrupted and wants to be descheduled
	INTERRUPTED
};

//! The Pipeline class represents an execution pipeline
class PipelineExecutor {
public:
	PipelineExecutor(ClientContext &context, Pipeline &pipeline);

	//! Fully execute a pipeline with a source and a sink until the source is completely exhausted
	PipelineExecuteResult Execute();
	//! Execute a pipeline with a source and a sink until finished, or until max_chunks have been processed
	//! Returns true if execution is finished, false if Execute should be called again
	PipelineExecuteResult Execute(idx_t max_chunks);

	//! Push a single input DataChunk into the pipeline.
	//! Returns either OperatorResultType::NEED_MORE_INPUT or OperatorResultType::FINISHED
	//! If OperatorResultType::FINISHED is returned, more input will not change the result anymore
	OperatorResultType ExecutePush(DataChunk &input);
	//! Called after depleting the source: finalizes the execution of this pipeline executor
	//! This should only be called once per PipelineExecutor
	void PushFinalize();

	//! Initializes a chunk with the types that will flow out of ExecutePull
	void InitializeChunk(DataChunk &chunk);
	//! Execute a pipeline without a sink, and retrieve a single DataChunk
	//! Returns an empty chunk when finished.
	void ExecutePull(DataChunk &result);
	//! Called after depleting the source using ExecutePull
	//! This flushes profiler states
	void PullFinalize();

	//! Allows the PipelineTask to propagate the interrupt up to the scheduler
	InterruptState& GetInterruptState() {
		return interrupt_state;
	}

private:
	//! The pipeline to process
	Pipeline &pipeline;
	//! The thread context of this executor
	ThreadContext thread;
	//! The total execution context of this executor
	ExecutionContext context;

	//! Intermediate chunks for the operators
	vector<unique_ptr<DataChunk>> intermediate_chunks;
	//! Intermediate states for the operators
	vector<unique_ptr<OperatorState>> intermediate_states;

	//! The local source state
	unique_ptr<LocalSourceState> local_source_state;
	//! The local sink state (if any)
	unique_ptr<LocalSinkState> local_sink_state;

	//! The final chunk used for moving data into the sink
	DataChunk final_chunk;

	//! The operators that are not yet finished executing and have data remaining
	//! If the stack of in_process_operators is empty, we fetch from the source instead
	stack<idx_t> in_process_operators;
	//! Whether or not the pipeline has been finalized (used for verification only)
	bool finalized = false;
	//! Whether or not the pipeline has finished processing
	int32_t finished_processing_idx = -1;
	//! Whether or not this pipeline requires keeping track of the batch index of the source
	bool requires_batch_index = false;
	//! State for the most recent operator interrupt will be stored here, this allows determining from the PipelineTask
	//! how to deschedule it.
	InterruptState interrupt_state;

	//! This flag is set when the pipeline gets interrupted by the Sink. It means that before continuing the pipeline
	//! we need to resink the last chunk
	bool remaining_sink_chunk = false;

	//! Flushing state
	bool started_flushing = false;
	bool done_flushing = false;

	idx_t flushing_idx;

private:
	void StartOperator(PhysicalOperator *op);
	void EndOperator(PhysicalOperator *op, DataChunk *chunk);

	//! Reset the operator index to the first operator
	void GoToSource(idx_t &current_idx, idx_t initial_idx);
	void FetchFromSource(DataChunk &result, bool allow_async);

	void FinishProcessing(int32_t operator_idx = -1);
	bool IsFinished();

	OperatorResultType ExecutePushInternal(DataChunk &input, idx_t initial_idx = 0);
	//! Pushes a chunk through the pipeline and returns a single result chunk
	//! Returns whether or not a new input chunk is needed, or whether or not we are finished
	OperatorResultType Execute(DataChunk &input, DataChunk &result, idx_t initial_index = 0);

	//! FlushCachedOperators methods push/pull any remaining cached results through the pipeline
	void FlushCachingOperatorsPull(DataChunk &result);

	//! Flushes a single chunk through the pipeline. May need to be called multiple times to flush all caching operators
	OperatorResultType FlushCachingOperatorsPush();

	static bool CanCacheType(const LogicalType &type);
	void CacheChunk(DataChunk &input, idx_t operator_idx);
};

} // namespace duckdb
