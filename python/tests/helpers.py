from linked_lists import Node


def chain(values):
    """Build a standalone chain of Nodes from an iterable; return its head."""
    head = tail = None
    for v in values:
        n = Node(v)
        if tail is None:
            head = tail = n
        else:
            tail.next = n
            tail = n
    return head


def to_list(head):
    """Walk a node chain and return its values as a list."""
    out = []
    while head:
        out.append(head.data)
        head = head.next
    return out
