TOOLS_PATH=./packages/Grpc.Tools.1.0.1/tools/windows_x64

$TOOLS_PATH/protoc.exe -I ../protos --csharp_out grpc --grpc_out grpc ../protos/pswmgr.proto --plugin=protoc-gen-grpc=$TOOLS_PATH/grpc_csharp_plugin.exe