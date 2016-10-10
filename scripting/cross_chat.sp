#include <rpc>

public Plugin myinfo = {
  name = "Cross Chat",
  author = "Dreae <dreae@dreae.onl>",
  description = "Simple SRPC example plugin",
  version = RPC_VERSION,
  url = "https://github.com/Dreae/sm-ext-rpc"
}

public void OnPluginStart() {
  RPCRegisterMethod("ChatMessage", chat_message, ParameterType:String, ParameterType:String, ParameterType:String);
}

public void chat_message(RPCContext context) {
  JSON params = context.GetParams();
  char from[256], steamid[64], message[256];
  params.GetArrayString(0, from, sizeof(from));
  params.GetArrayString(1, steamid, sizeof(steamid));
  params.GetArrayString(2, message, sizeof(message));
  params.Close();

  char formated[1028];
  Format(formated, sizeof(formated), "%s %s - %s", from, steamid, message);

  SayText2(formated);
  
  context.Done();
}

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

public Action OnClientSayCommand(int client, const char[] command, const char[] sArgs) {
  char cmd[32];
  strcopy(cmd, sizeof(cmd), command);
  TrimString(cmd);
  if(StrEqual(cmd, "say", false)) {
    char steamid[64], name[256];

    GetClientAuthId(client, AuthId_Steam3, steamid, sizeof(steamid));
    GetClientName(client, name, sizeof(name));    

    RPCCall call = new RPCCall();
    call.SetMethod("ChatMessage");

    JSON jArgs = new JSON();
    jArgs.PushString(name);
    jArgs.PushString(steamid);
    jArgs.PushString(sArgs);

    call.SetParams(jArgs);
    jArgs.Close();

    call.Broadcast();
  }
}