# pswmgr
Password Manager that uses a client/server architecture to store encrypted passwords remotely

# Dependencies
* gRpc
* Protocol Buffers
* OpenSSL
* Sqlite3 (available in repos)

# Supported platforms
* Windows (GUI)
* Linux
 * Server
 * Command Line Interface
* Android (GUI)

First Header  | Second Header
------------- | -------------
Content Cell  | Content Cell
Content Cell  | Content Cell

         |  GUI  |  CLI  | Server |
-------- | ----- | ----- | ------ |
Windows  | _YES_ | _YES_ |  _YES_ |
Linux    |   NO  | _YES_ |  _YES_ |
Android  | _YES_ |   NO  |   NO   |
