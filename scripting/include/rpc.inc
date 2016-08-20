#if defined _rpc_included
  #endinput
#endif
#define _rpc_included

enum ParameterType {
  String = 1,
  Int,
  Bool
}

methodmap RPCContext < Handle {
  public native int GetParamInt(int pos);

  public native float GetParamFloat(int pos);

  public native bool GetParamBool(int pos);

  public native void GetParamString(int pos, char[] buffer, int maxlen);

  public native void SetReturnInt(int value);

  public native void SetReturnFloat(float value);

  public native void SetReturnBool(bool value);

  public native void SetReturnString(char[] value);

  public native void SetReturnJSON(JSON value);

  public native void Done();
}

methodmap JSON < Handle {
  public native JSON();

  public native void SetInt(char[] key, int value);

  public native void SetFloat(char[] key, float value);

  public native void SetBool(char[] key, bool value);

  public native void SetString(char[] key, char[] value);

  public native void SetJSON(char[] key, JSON value);

  public native int GetInt(char[] key);

  public native float GetFloat(char[] key);

  public native bool GetBool(char[] key);

  public native void GetString(char[] key, char[] buffer, int maxlen);
}

methodmap RPCCall < Handle {
  public native RPCCall(RPCReplyCallback callback);

  public native void SetMethod(char[] method);

  public native void SetParamsJSON(JSON params);

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
