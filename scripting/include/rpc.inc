#if defined _rpc_included
  #endinput
#endif
#define _rpc_included
#define RPC_VERSION "1.0.0"

enum ParameterType {
  String = 1,
  Int,
  Bool
}

methodmap RPCContext < Handle {
  public native JSON GetParams()

  public native void SetReturn(JSON value);

  public native void Done();
}

methodmap JSON < Handle {
  public native JSON();

  public native void SetInt(char[] key, int value);

  public native void SetFloat(char[] key, float value);

  public native void SetBool(char[] key, bool value);

  public native void SetString(char[] key, char[] value);

  public native void SetJSON(char[] key, JSON value);

  public native void PushString(char[] value);

  public native void PushInt(int value);

  public native void PushFloat(float value);

  public native void PushBool(bool value);

  public native void PushJSON(JSON value);

  public native int GetInt(char[] key);

  public native float GetFloat(char[] key);

  public native bool GetBool(char[] key);

  public native JSON GetJSON(char[] key);

  public native void GetString(char[] key, char[] buffer, int maxlen);

  public native void GetArrayString(int pos, char[] buffer, int maxlen);

  public native int GetArrayInt(int pos);

  public native bool GetArrayBool(int pos);

  public native float GetArrayFloat(int pos);

  public native JSON GetArrayJSON(int pos);
}

methodmap RPCCall < Handle {
  public native RPCCall(RPCReplyCallback callback);

  public native void SetMethod(char[] method);

  public native void SetParams(JSON params);

  public native void Notify(char[] server);

  public native void Send(char[] server);

  public native void Broadcast();
}

typedef RPCCallback = function void (RPCContext context);
typedef RPCReplyCallback = function void (JSON result);

native RPCRegisterMethod(char[] name, RPCCallback callback, ParameterType ...);

public Extension:__ext_smsock = {
	name = "RPC",
	file = "rpc.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};
