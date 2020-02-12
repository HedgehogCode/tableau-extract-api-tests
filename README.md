# Test tableau extract API access with multiple threads.

This repository demonstrates problems with the tableau extract api if it is accessed by multiple threads at the same time. The files in the folder `extractapi-linux-x86_64-2019_2` and `lib` have been downloaded from https://help.tableau.com/current/api/extract_api/en-us/Extract/extract_api_installing.htm.

## Getting Started

* Import the project into eclipse
* Create a new run configuration for the main class `StresstestExtractAPI`
* Add the `LD_LIBRARY_PATH` environment variable with the value `${project_loc}/extractapi-linux-x86_64-2019_2/lib64/tableausdk` to the run configuration
* Run the run configuraion

## Observed Errors

There are two errors that appear sometimes. Run the application multiple times to observe both error. The errors appeared on Ubuntu 19.10.

### Invalid collation name

```
Caused by: com.tableausoftware.TableauException: invalid collation name
	at com.tableausoftware.hyperextract.TableDefinition.addColumn(Unknown Source)
	at StresstestExtractAPI.writeHyperFile(StresstestExtractAPI.java:63)
	at StresstestExtractAPI.access$0(StresstestExtractAPI.java:58)
	at StresstestExtractAPI$ExtractWriter.run(StresstestExtractAPI.java:90)
	... 5 more
```

### std::_Rb_tree_insert_and_rebalance

```
#
# A fatal error has been detected by the Java Runtime Environment:
#
#  SIGSEGV (0xb) at pc=0x00007f6ad849a13a, pid=26362, tid=0x00007f6a5ecc0700
#
# JRE version: OpenJDK Runtime Environment (8.0_242-b08) (build 1.8.0_242-8u242-b08-0ubuntu3~19.10-b08)
# Java VM: OpenJDK 64-Bit Server VM (25.242-b08 mixed mode linux-amd64 compressed oops)
# Problematic frame:
# [thread 140094836848384 also had an error]
C  [libstdc++.so.6+0xa713a]  std::_Rb_tree_insert_and_rebalance(bool, std::_Rb_tree_node_base*, std::_Rb_tree_node_base*, std::_Rb_tree_node_base&)+0xca
#
# Failed to write core dump. Core dumps have been disabled. To enable core dumping, try "ulimit -c unlimited" before starting Java again
#
# An error report file with more information is saved as:
# /home/benjamin/eclipse-workspace/tableau-extract-api-tests/hs_err_pid26362.log
#
# If you would like to submit a bug report, please visit:
#   http://bugreport.java.com/bugreport/crash.jsp
# The crash happened outside the Java Virtual Machine in native code.
# See problematic frame for where to report the bug.
#
```
