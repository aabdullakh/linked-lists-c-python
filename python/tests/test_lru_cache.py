import pytest

from linked_lists import LRUCache


def test_capacity_must_be_positive():
    for bad in (0, -1):
        with pytest.raises(ValueError):
            LRUCache(bad)


def test_get_missing_returns_default():
    c = LRUCache(2)
    assert c.get("x") is None
    assert c.get("x", "fallback") == "fallback"


def test_put_then_get():
    c = LRUCache(2)
    c.put("a", 1)
    assert c.get("a") == 1
    assert len(c) == 1 and "a" in c


def test_evicts_least_recently_used_when_full():
    c = LRUCache(2)
    c.put("a", 1); c.put("b", 2)
    c.put("c", 3)                                  # evicts "a" (oldest, never read)
    assert c.get("a") is None
    assert c.get("b") == 2 and c.get("c") == 3
    assert len(c) == 2


def test_get_refreshes_recency():
    c = LRUCache(2)
    c.put("a", 1); c.put("b", 2)
    assert c.get("a") == 1                         # "a" is now newest
    c.put("c", 3)                                  # so "b" is evicted
    assert c.get("b") is None
    assert c.get("a") == 1 and c.get("c") == 3


def test_put_existing_key_updates_value_and_recency_without_evicting():
    c = LRUCache(2)
    c.put("a", 1); c.put("b", 2)
    c.put("a", 10)                                 # update, no eviction
    assert len(c) == 2
    c.put("c", 3)                                  # evicts "b", not "a"
    assert c.get("b") is None
    assert c.get("a") == 10


def test_capacity_one():
    c = LRUCache(1)
    c.put("a", 1)
    c.put("b", 2)
    assert c.get("a") is None and c.get("b") == 2
    assert len(c) == 1


def test_keys_are_listed_most_recent_first():
    c = LRUCache(3)
    c.put("a", 1); c.put("b", 2); c.put("c", 3)
    c.get("a")
    assert list(c.keys()) == ["a", "c", "b"]


def test_contains_does_not_refresh_recency():
    c = LRUCache(2)
    c.put("a", 1); c.put("b", 2)
    assert "a" in c                                # membership test only
    c.put("c", 3)
    assert "a" not in c                            # still evicted as the LRU


def test_stores_falsy_values_and_none_values():
    c = LRUCache(2)
    c.put("zero", 0)
    c.put("none", None)
    assert c.get("zero", "miss") == 0
    assert c.get("none", "miss") is None           # a stored None is a hit


def test_map_and_list_stay_in_sync_under_load():
    c = LRUCache(50)
    for i in range(2000):
        c.put(i % 80, i)
        if i % 3 == 0:
            c.get((i * 7) % 80)
        assert len(c.map) == len(c.order) <= 50
        assert set(c.map) == set(c.keys())
