# pswmgr
Password Manager that uses a client/server architecture to store encrypted passwords remotely

# Dependencies
* gRpc
* Protocol Buffers
* OpenSSL
* Sqlite3 (available in repos)

# Supported platforms

         |  GUI  |  CLI  | Server |
-------- | ----- | ----- | ------ |
Linux    |   NO  |__YES__| __YES__|
Windows  | _YES_ |  NO   |   NO   |

# Important notice
Currently the storage of passwords is done in plain text. This means that the current implementation is __NOT SECURE__