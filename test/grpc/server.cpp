#include "hello.grpc.pb.h"
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include <grpc++/support/status.h>
#include <iostream>
#include <sstream>

using namespace hello;

class HelloServiceImpl : public Hello::Service {
  ::grpc::Status SayHello(::grpc::ServerContext* context,
                          const ::HelloRequest* request,
                          ::HelloResponse* response) override {
    std::cout << "RECEIVED : " << request->name() << std::endl;
    response->set_greet(std::string("Hello ") + request->name());
    return grpc::Status::OK;
  }

  ::grpc::Status BatchHello(::grpc::ServerContext* context,
                            ::grpc::ServerReader<::HelloRequest>* reader,
                            ::HelloResponse* response) override {
    std::stringstream ss;
    ss << "Hello";
    HelloRequest request;
    while (reader->Read(&request)) {
      std::cout << "RECEIVED : " << request.name() << std::endl;
      ss << " " << request.name();
    }
    std::cout << "RECEIVED END" << std::endl;
    response->set_greet(ss.str());
    return grpc::Status::OK;
  }
};

void run() {
  std::string addr("0.0.0.0:41384");
  grpc::ServerBuilder builder;
  HelloServiceImpl service;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}

int main(int, char**) {
  grpc_init();
  run();
  grpc_shutdown();
  return 0;
}
