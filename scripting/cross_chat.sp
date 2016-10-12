#include <rpc>

public Plugin myinfo = {
  name = "Cross Chat",
  author = "Dreae <dreae@dreae.onl>",
  description = "Simple SRPC example plugin",
  version = RPC_VERSION,
  url = "https://github.com/Dreae/sm-ext-rpc"
}

public void OnPluginStart() {
  // Register an RPC method named ChatMessage. This will be the name used by other
  // plugins to call this method.
  RPCRegisterMethod("ChatMessage", chat_message, ParameterType:String, ParameterType:String, ParameterType:String);
}

// Define the method callback. This function will get called
// when other plugins make an RPC call to the method you
// defined at plugin start.
public void chat_message(RPCContext context) {
  // Get a JSON object representing the parameters to this call
  JSON params = context.GetParams();
  char from[256], steamid[64], message[256];

  // Get the values passed to this call and store them in local variables
  params.GetArrayString(0, from, sizeof(from));
  params.GetArrayString(1, steamid, sizeof(steamid));
  params.GetArrayString(2, message, sizeof(message));

  // Close the JSON object handle now that we're done with it.
  params.Close();

  char formated[1028];
  Format(formated, sizeof(formated), "%s %s - %s", from, steamid, message);

  // Print the message we recieved from the other server
  SayText2(formated);
  
  // Close the call context to clean up
  context.Done();
}

public Action OnClientSayCommand(int client, const char[] command, const char[] sArgs) {
  char cmd[32];
  strcopy(cmd, sizeof(cmd), command);
  TrimString(cmd);
  if(StrEqual(cmd, "say", false)) {
    char steamid[64], name[256];

    GetClientAuthId(client, AuthId_Steam3, steamid, sizeof(steamid));
    GetClientName(client, name, sizeof(name));    

    // Start a new RPC call and set the name of the method we're calling
    RPCCall call = new RPCCall();
    call.SetMethod("ChatMessage");

    // Create a new JSON object for storing our parameters
    JSON jArgs = new JSON();

    // Push the users name, Steam ID, and they message they're sending onto
    // our parameters JSON array.
    jArgs.PushString(name);
    jArgs.PushString(steamid);
    jArgs.PushString(sArgs);

    // Set the parameters for the call to the JSON object we created
    call.SetParams(jArgs);
    // Close our JSON object to clean up, call.SetParams makes a copy
    jArgs.Close();

    // Broadcast our call to all defined servers
    call.Broadcast();
  }
}

// This is just a helper method for printing to chat with colors
void SayText2(char[] message) {
  new Handle:pb = StartMessageAll("SayText2");
  if (pb != INVALID_HANDLE) {
    if(GetUserMessageType() == UM_Protobuf) {
      PbSetInt(pb, "ent_idx", 0);
      PbSetBool(pb, "chat", true);

      PbSetString(pb, "msg_name", message);
      PbAddString(pb, "params", "");
      PbAddString(pb, "params", "");
      PbAddString(pb, "params", "");
      PbAddString(pb, "params", "");

      EndMessage();
    } else {
      BfWriteByte(pb, 0);
      BfWriteByte(pb, true);
      BfWriteString(pb, message);
      EndMessage();
    }
  }
}