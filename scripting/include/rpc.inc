#if defined _rpc_included
  #endinput
#endif
#define _rpc_included
#define RPC_VERSION "1.0.0"

enum ParameterType {
  String = 1,
  Int,
  Bool,
  Float,
  Json
}

methodmap RPCContext < Handle {
  // Gets a JSON object representing the parameters passed to this
  // RPC method by the calling server over the network. Remember to
  // close the handle returned from this method.
  // 
  // @return              Handle to the parameters JSON object.
  public native JSON GetParams()

  // Sets the return value for this RPC method. This value will be
  // returned to the calling server.
  // 
  // @param value         Handle to a JSON object to be returned
  //                      to the calling server. This object is
  //                      copied, the original value will still
  //                      need to be closed.
  public native void SetReturn(JSON value);

  // Gets the address of the remote server as a string
  //
  // @param address       The buffer to store the address in
  // @param maxSize       The maximum size of the buffer
  public native void GetRemoteAddress(char[] buffer, int maxSize);

  // Signals that you are done handling this RPC request. Calling this
  // method closes the RPCContext handle, and sends the return value
  // to the calling server if the call was not a Notify or Broadcast
  public native void Done();
};

methodmap JSON < Handle {
  // Creates a new JSON object. This object represents either a JSON array
  // or a JSON object.
  // 
  // @return              A handle to the new JSON object.
  public native JSON();

  // Sets the int value of the JSON object represented by this 
  // handle at the specified key.
  // 
  // @param key           A string representing the key to set.
  // @param value         The value to store in the JSON object.
  public native void SetInt(const char[] key, int value);

  // Sets the float value of the JSON object represented by this 
  // handle at the specified key.
  // 
  // @param key           A string representing the key to set.
  // @param value         The value to store in the JSON object.
  public native void SetFloat(const char[] key, float value);

  // Sets the bool value of the JSON object represented by this 
  // handle at the specified key.
  // 
  // @param key           A string representing the key to set.
  // @param value         The value to store in the JSON object.
  public native void SetBool(const char[] key, bool value);

  // Sets the string value of the JSON object represented by this 
  // handle at the specified key.
  // 
  // @param key           A string representing the key to set.
  // @param value         The value to store in the JSON object.
  //                      The string value of this method is copied.
  public native void SetString(const char[] key, const char[] value);

  // Sets the JSON value of the JSON object represented by this 
  // handle at the specified key.
  // 
  // @param key           A string representing the key to set.
  // @param value         The value to store in the JSON object.
  //                      The object pointed to by this handle is
  //                      copied
  public native void SetJSON(const char[] key, JSON value);

  // Pushes a string value onto the JSON array represented by this 
  // handle
  // 
  // @param value         The string value to be pushed onto the
  //                      array
  public native void PushString(const char[] value);

  // Pushes an int value onto the JSON array represented by this 
  // handle
  // 
  // @param value         The int value to be pushed onto the
  //                      array
  public native void PushInt(int value);

  // Pushes a float value onto the JSON array represented by this 
  // handle
  // 
  // @param value         The float value to be pushed onto the
  //                      array
  public native void PushFloat(float value);

  // Pushes a bool value onto the JSON array represented by this 
  // handle
  // 
  // @param value         The bool value to be pushed onto the
  //                      array
  public native void PushBool(bool value);

  // Pushes a JSON value onto the JSON array represented by this 
  // handle
  // 
  // @param value         The JSON value to be pushed onto the
  //                      array. The object pointed to by this
  //                      handle is copied.
  public native void PushJSON(JSON value);

  // Gets an int value from the JSON object represented by this
  // handle
  // 
  // @param key           The key to look up
  // @return              The value at object[key]
  public native int GetInt(const char[] key);

  // Gets a float value from the JSON object represented by this
  // handle
  // 
  // @param key           The key to look up
  // @return              The value at object[key]
  public native float GetFloat(const char[] key);

  // Gets a bool value from the JSON object represented by this
  // handle
  // 
  // @param key           The key to look up
  // @return              The value at object[key]
  public native bool GetBool(const char[] key);

  // Gets a JSON value from the JSON object represented by this
  // handle
  // 
  // @param key           The key to look up
  // @return              The value at object[key]
  public native JSON GetJSON(const char[] key);

  // Gets a string value from the JSON object represented by this
  // handle
  // 
  // @param key           The key to look up
  // @param buffer        The buffer to store the string value 
  //                      at object[key]
  // @param maxlen        The maximum size of the buffer
  public native void GetString(const char[] key, char[] buffer, int maxlen);

  // Gets a string value at pos from the JSON array represented by
  // this handle
  // 
  // @param pos           The position to fetch from the array
  // @param buffer        The buffer to store the fetched string
  // @param maxlen        The maximum size of the buffer
  public native void GetArrayString(int pos, char[] buffer, int maxlen);

  // Gets an int value at pos from the JSON array represented by
  // this handle
  // 
  // @param pos           The position to fetch from the array
  // @return              The value at array[pos]
  public native int GetArrayInt(int pos);

  // Gets a bool value at pos from the JSON array represented by
  // this handle
  // 
  // @param pos           The position to fetch from the array
  // @return              The value at array[pos]
  public native bool GetArrayBool(int pos);

  // Gets a float value at pos from the JSON array represented by
  // this handle
  // 
  // @param pos           The position to fetch from the array
  // @return              The value at array[pos]
  public native float GetArrayFloat(int pos);

  // Gets a JSON value at pos from the JSON array represented by
  // this handle
  // 
  // @param pos           The position to fetch from the array
  // @return              The value at array[pos]
  public native JSON GetArrayJSON(int pos);

  // Gets the size of the JSON array represented by this handle
  // 
  // @return              The size of this JSON array 
  public native int GetArraySize();
};

methodmap RPCCall < Handle {
  // Creates a new RPCCall. This handle is used to make calls
  // to methods defined on over servers.
  // 
  // @return              A handle to the new RPCCall object
  public native RPCCall();

  // Sets the name of the method this object will call
  // 
  // @param method        The name of the foreign method to call
  public native void SetMethod(const char[] method);

  // Sets the parameters of this method
  // 
  // @param params        A JSON object to be sent as parameters to
  //                      this method. The JSON object is copied.
  public native void SetParams(JSON params);

  // Send this method call as a notification to the foreign server.
  // Unlike Send, notify expects no return values and has no way to
  // receive a response from the other server.
  // 
  // @param server        The server name to send the notification to
  public native void Notify(const char[] server);

  // Sends this method call to a foreign server, and sets the callback
  // to handle to call results.
  // 
  // @param server        The server name to send the call to
  // @param callback      The callback function for handling the results
  //                      from the foreign server
  public native void Send(const char[] server, RPCReplyCallback callback);

  // Broadcasts this method call to all known servers. Like Notify, this
  // method has no way of handling responses from individual calls.
  public native void Broadcast();
};

typedef RPCCallback = function void (RPCContext context);
typedef RPCReplyCallback = function void (JSON result);

/**
 * Registers a new RPC method that will be available on this server
 * 
 * @param name          The name of this RPC method. This name will be
 *                      used by plugins wishing to call this method
 * @param callback      The function that will be called when this server
 *                      receives a foreign call for this method
 * @param ...           A variable number of ParameterTypes. These represent
 *                      the types of the arguments expected by this method so
 *                      that SRPC can validate the request before calling the
 *                      callback function.
 */
native void RPCRegisterMethod(const char[] name, RPCCallback callback, ParameterType ...);

/**
 * Gets a JSON array with the names of all known servers.
 * 
 * @return              A JSON array containing the names of all servers
 *                      as strings
 */
native JSON RPCGetServers();

/**
 * Registers a new server for making RPC calls.
 *
 * @param name          The name the new server will be registered as, used
 *                      for making calls to this server
 * @param address       The IP address of the new server
 * @param port          The port of the new server
 */
native void RPCAddServer(const char[] name, const char[] address, int port);

/**
 * Removes a server from the list of known servers.
 *
 * @param name          The name of the server to remove.
 */
native void RPCRemoveServer(const char[] name);

/**
 * Gets the port of a known server
 *
 * @param name          The name of the server to look up the port of
 * @return              The port the named server is listening on
 */
native int RPCGetServerPort(const char[] name);

/**
 * Gets the address of a known server
 *
 * @param name          The name of the server to look up the address of
 * @param address       The buffer to store the address in
 * @param maxSize       The maximum size of the buffer
 */
native void RPCGetServerAddress(const char[] name, char[] address, int maxSize);

/**
 * Gets the listen port of the local server
 *
 * @return              The port the current server is listening on
 */
native int RPCGetLocalPort();

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
