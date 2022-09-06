@0x957d93d69e1d06f6;
using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("Cap");

  

struct PutRecordRequest {
    topic @0 :Text;
    sizeOfRecordsComming @1: UInt64;
}

struct GetRecordRequest {
    topic @0 :Text;
    isStartTime @1: Bool;
    startTime @2: UInt64;
    isEndTime @3: Bool;
    endTime @4: UInt64;
}

struct Message {
  payload : union {
    messagePut @0: PutRecordRequest;
    messageGet @1: GetRecordRequest;
  }
}