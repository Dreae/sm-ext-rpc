#include <sourcemod>
#include <rpc>

public Plugin myinfo = {
  name = "RPC Test",
  author = "Dreae <dreae@dreae.onl>",
  description = "Used for testing SRPC",
  version = RPC_VERSION,
  url = "https://github.com/Dreae/sm-ext-rpc"
}

public void OnPluginStart() {
  RegServerCmd("rpc_test", rpcTest);
  RegServerCmd("rpc_print_servers", rpcPrintServers);
  RegServerCmd("rpc_broadcast", rpcBroadcast);
  RPCRegisterMethod("TestMethod", commandCallback, ParameterType:String);
}

public Action rpcTest(int args) {
  char argString[256];
  GetCmdArgString(argString, sizeof(argString));

  RPCCall call = new RPCCall();
  call.SetMethod("TestMethod");
  
  JSON jArgs = new JSON();
  jArgs.PushString(argString);
  call.SetParams(jArgs);
  jArgs.Close();
  call.Send("test", replyCallback);
}

public Action rpcPrintServers(int args) {
  JSON servers = RPCGetServers();

  for(int i = 0; i < servers.GetArraySize(); i++) {
    char serverName[256];
    servers.GetArrayString(i, serverName, sizeof(serverName));

    PrintToServer("Server %d: %s", i, serverName);
  }

  servers.Close();
}

public Action rpcBroadcast(int args) {
  char argString[256];
  GetCmdArgString(argString, sizeof(argString));

  RPCCall call = new RPCCall();
  call.SetMethod("TestMethod");
  
  JSON jArgs = new JSON();
  jArgs.PushString(argString);
  call.SetParams(jArgs);
  jArgs.Close();
  call.Broadcast();
}

public void replyCallback(JSON result) {
  char reply[256];
  result.GetString("message", reply, sizeof(reply));
  PrintToServer("Got reply from test command: %s", reply);

  result.Close();
}

public void commandCallback(RPCContext context) {
  JSON params = context.GetParams();
  char message[256];
  params.GetArrayString(0, message, sizeof(message));
  params.Close();
  
  PrintToServer("Got called by test server with message: %s", message);
  JSON ret = new JSON();
  ret.SetString("message", message);
  context.SetReturn(ret);
  context.Done();

  ret.Close();
}