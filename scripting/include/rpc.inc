#if defined _rpc_included
  #endinput
#endif
#define _rpc_included

enum ParameterType {
  String = 1,
  Int,
  Bool
}

native RPCRegisterMethod(char[] name, ParameterType returnType, ParameterType ...);

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
