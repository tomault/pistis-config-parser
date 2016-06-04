# pistis-config-parser
Configuration file parser.  Parses simple configuration files and maps them to an object.  Configuration files look like:

\# This is a comment

\# Parse the configuration in "some_file" as if it were part of this file
include "some_file"

\# Set the value of configuration properties
some_property = value
foo.bar.baz = some_value

\# Same as:
\#   some_group.property_1 = 1
\#   some_group.property_2 = foo
some_group {
  property_1 = 1
  property_2 = foo
}

