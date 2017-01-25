# pswmgr
Password Manager that uses a client/server architecture to store encrypted passwords remotely

# Dependencies
* [gRpc](http://www.grpc.io/)
* [Protocol Buffers](https://developers.google.com/protocol-buffers/)
* [OpenSSL](https://www.openssl.org/)
* [Sqlite3 (available in repos)](https://sqlite.org/)

# Supported platforms

         |  GUI  |  CLI  | Server |
-------- | ----- | ----- | ------ |
Linux    |   NO  |__YES__| __YES__|
Windows  |__YES__|  NO   |   NO   |

# Important notice
Currently the storage of passwords is done in plain text. This means that the current implementation is __NOT SECURE__