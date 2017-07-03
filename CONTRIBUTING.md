# Contribution Guideline and Coding style guide for FlexCore Development

The first rule of the Contribution Guideline is: Contribute!  
We are confident that stylistic inconsitencies etc. can easily be fixed.
This guideline is meant to encourage contribution, not block it.

## C++

We generally try to adher to the [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).

The coding style in general is meant to be close to that of the stl and boost,
since flexcore is used as a foundation library for other domain specific libraries and tools.

### Code layout
* Lines are strictly limited to 100 characters
* Identation is done with tabs and tab width is 4 characters
* namespace content is not indented
* Access labels (public, protected, private) do not add an indent level
* [Allman style](https://en.wikipedia.org/wiki/Indent_style#Allman_style) for curly brackets
* Use of whitespaces within a line are left to the coder and shall enhance readability

### Identifier
* The language used for identifiers and comments is english.
* Identifiers:
  * types and objects are all lower case
  * MACROS and only MACROS are all caps.
  * Concepts start with an uppercase letter
  * Template parameters and alias types commonly have a _t suffix
* Distinction between types, objects, members, nonmembers is the responsibility of the UI, not of the use of upper case characters 

### Comments
* There is no documentation header in files
* The language used for identifiers and comments is english.
* Comments describing an API should are to be in doxygen format
* If functions have preconditions or postconditions they need to explained in their documentation
* Classes should have documentation explaining their purpose, use and if helpful an example
* Comments are required where they are needed for clarification **but** should not be present where they add no additional information

### Other
* Header files have the extension .hpp, Source files .cpp
* Includes are grouped (in this order) in: flexcore includes, standard library, boost, other system libraries.
* Includes may not use "." or ".."