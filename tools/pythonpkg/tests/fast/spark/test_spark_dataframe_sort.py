import pytest

from duckdb.experimental.spark.sql.types import Row

_ = pytest.importorskip("duckdb.experimental.spark")


class TestDataFrameSort(object):
    data = [(56, "Carol"), (20, "Alice"), (3, "Dave"), (3, "Anna"), (1, "Ben")]

    def test_sort_ascending(self, spark):
        df = spark.createDataFrame(self.data, ["age", "name"])
        expected = [
            Row(age=1, name="Ben"),
            Row(age=3, name="Anna"),
            Row(age=3, name="Dave"),
            Row(age=20, name="Alice"),
            Row(age=56, name="Carol"),
        ]

        df = df.sort(["age", "name"])
        assert df.collect() == expected

        df = df.sort("age", "name")
        assert df.collect() == expected

        df = df.sort(1, 2)
        assert df.collect() == expected

    def test_sort_descending(self, spark):
        df = spark.createDataFrame(self.data, ["age", "name"])
        expected = [
            Row(age=20, name="Alice"),
            Row(age=3, name="Anna"),
            Row(age=1, name="Ben"),
            Row(age=56, name="Carol"),
            Row(age=3, name="Dave"),
        ]

        df = df.sort(["name", "age"])
        assert df.collect() == expected

        df = df.sort("name", "age")
        assert df.collect() == expected

        df = df.sort(2, 1)
        assert df.collect() == expected
