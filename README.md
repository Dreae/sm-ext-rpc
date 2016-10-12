# SRPC

SRPC provides remote procedure call to sourcemod plugins, enabling easy
networking between plugins running on different servers or external services.

## Configuration

Before using SRPC it is important to configure where the extension should listen
for incoming procedure calls, as well as providing a list of additional servers.

```json
{
  "address": "0.0.0.0",
  "port": 1337,
  "secret": "api_key",
  "servers": {
    "server1": {
      "address": "192.168.0.3",
      "port": 1337
    }
  }
}
```

In the above example `address` and `port` configure where the local SRPC will
listen for incoming requests. `secret` specifies the request signing key. This
key will be used to sign all RPC request to prevent unauthorized calls being made,
this value MUST be the same for all servers. The `servers` object provides server
names and addresses for servers this server may call. The server name is how plugins
will make calls to that server.

## Usage

Below is an example for using SRPC to make simple calls between servers. For more 
usage examples please see the annoted code in `cross_chat.sp` in the scripting 
directory.

```SourcePawn
#include <rpc>

public Plugin myinfo = {
  name = "SRPC Echo",
  author = "Dreae <dreae@dreae.onl>",
  description = "Simple SRPC example plugin",
  version = RPC_VERSION,
  url = "https://github.com/Dreae/sm-ext-rpc"
}

public void OnPluginStart() {
  // Register an RPC method named Echo, this will be the name used by other
  // plugins to call this method, and specify that our method takes a String
  // argument
  RPCRegisterMethod("Echo", echo, ParameterType:String);
  // Register a server command for triggering an echo
  RegServerCmd("rpc_echo", echo_cmd);
}

public Action echo_cmd(int args) {
  char argString[256];
  // Get the full command argument string to be echoed back
  GetCmdArgString(argString, sizeof(argString));

  // Start a new RPCCall, and define the method name we will be calling
  RPCCall call = new RPCCall();
  call.SetMethod("Echo");
  
  // Create a new JSON object to hold our parameters.
  JSON jArgs = new JSON();
  // Push our echo string to the parameter array
  jArgs.PushString(argString);
  // Set the call to use our JSON parameters
  call.SetParams(jArgs);
  // Close our JSON object, call.SetParams makes a copy, so it's no longer needed
  jArgs.Close();

  // Send our call to the server named "test" and set a callback for handling that
  // server's reply
  call.Send("test", echo_callback);
}

// RPC callback, the results set by the other server are passed as a JSON object
public void echo_callback(JSON result) {
  char reply[256];
  // Get the "message" value returned by the other server and print it.
  result.GetString("message", reply, sizeof(reply));
  PrintToServer("Got reply from test command: %s", reply);

  // Close the results JSON object to clean up.
  result.Close();
}

// RPC command callback. This function will get called when another server calls
// our Echo method.
public void echo(RPCContext context) {
  // Get a JSON object representing our method arguments
  JSON params = context.GetParams();
  char message[256];

  // Get the first argument we were passed as a string
  params.GetArrayString(0, message, sizeof(message));
  // Close our parameter object to clean up.
  params.Close();
  
  PrintToServer("Got called by test server with message: %s", message);

  // Create a new JSON object to hold our reply to the calling server
  JSON ret = new JSON();
  // Set the "message" value on our reply to the string we were given
  ret.SetString("message", message);
  // Set the return value for this RPC request to the JSON return value
  context.SetReturn(ret);
  // Close the JSON return object we created. SetReturn creates a copy.
  ret.Close();

  // Signal that we are done processing the request, and that the return value
  // should be sent back to the calling server.
  // NOTE: context is automatically closed when you call Done();
  context.Done();
}
```
