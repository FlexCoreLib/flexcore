#Rationale

The main goal of FlexCore is to provide a library, which allows programmers to easily design and code dataflow programs, which are as fast and robust as expected by any good C++ program.

To achieve this we chose a template based design as this allows developers to enjoy the full support of the C++ type system and still achieve Genericity and Extensibility without adding unnecessary burdens to runtime performance. Staying within the bounds of a statically typed system moves as many error checks as possible from runtime to compilation. This increases robustness of the resulting program.
FlexCore does not restrict programmers to a specific problem domain or makes them jump through hoops to extend it with their own code.

We developed FlexCore with applications in Sensor Technology and Automation in mind.
This context influences our design goals, which where:
* Genericity: FlexCore makes as few assumptions about the algorithms and data-types developed and used by client code.
* Robustness: FlexCore enables a lot of compile time error checks by sticking to the full C++ type system.
* Real-time Capability: FlexCore does not contain any hidden performance traps. The template based design does not hide the execution path behind unnecessary indirection and thus allows compilers to optimize the whole dataflow program.
* Extensibility: The generic design runs through the whole library and allows users to easily extend it by specifying their own nodes, ports and connectables.

Unfortunately, one notable drawback of this approach is the lack of readability of the compiler errors in many cases.
We believe that the benefits outweigh this problem and are confident, that the fast moving support for generic programming in modern C++ will continuously improve this situation.

Mitigation strategies:
* As many concept checks and static asserts as reasonable possible. Many cases are already checked as close to the user code as possible. Merge Requests with more and better checks are always welcome.
* Compile with a different compiler. Error messages for template code are quickly becoming better. Try to compile the program with the newest version of gcc and clang.