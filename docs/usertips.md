# Tips and Guidelines for using FlexCore in your own code base

1. **Prefer Connectables over Nodes**

    If a job can be done by a connectables, don't implement a full fledged node.
    Connectables are more generic and allow the compiler to generate faster code.
    Connectables also make it easier to reason about code as the dataflow stays linear and is clearly visible in the code.

    Connectables can simply be aggregated to nodes if it proves necessary.
    If a connectables produces more than one output, it might be useful to return all outputs as an std::tuple and process them together.

2. **Prefer to make Nodes and Connectables pure**

    If a Node or Connectable can perform it's responsibility without internal state, let it do so. This makes it easer to reason about the correctness of the code.

3. **Make `operator >>` available by `using fc::operator>>;`**

    As soon as one connects two objects which are not from namespace fc, `operator>>` needs to be made available.
Prefer to import only the operator instead of `using namespace fc` to minimize number of symbols imported.
