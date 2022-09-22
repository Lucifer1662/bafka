@0x957d93d69e1d06f6;
using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("Cap");

  

struct PutRecordRequest {
    topic @0 :Text;
    sizeOfRecordsComming @1: UInt64;
}

struct GetRecordRequest {
    topic @0 :Text;
    messageId @1: UInt64;
    bufferSize @2: UInt64;
}

struct Message {
  payload : union {
    messagePut @0: PutRecordRequest;
    messageGet @1: GetRecordRequest;
  }
}

struct Record {
  eventName @0 : Text;
  time @1 : UInt64;
}