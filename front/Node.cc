#include "Node.h"

Node::Node(Tag t)
{
    tag = t;
}

std::string Node::ToString() const
{
    return std::to_string(static_cast<int>(tag));
}

Tag Node::GetTag() const
{
    return tag;
}
