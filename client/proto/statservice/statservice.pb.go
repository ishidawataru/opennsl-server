// Code generated by protoc-gen-go.
// source: statservice.proto
// DO NOT EDIT!

/*
Package statservice is a generated protocol buffer package.

It is generated from these files:
	statservice.proto

It has these top-level messages:
*/
package statservice

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import stat "github.com/ishidawataru/opennsl-server/client/proto/stat"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion3

// Client API for Stat service

type StatClient interface {
	Init(ctx context.Context, in *stat.InitRequest, opts ...grpc.CallOption) (*stat.InitResponse, error)
	Clear(ctx context.Context, in *stat.ClearRequest, opts ...grpc.CallOption) (*stat.ClearResponse, error)
	Sync(ctx context.Context, in *stat.SyncRequest, opts ...grpc.CallOption) (*stat.SyncResponse, error)
	Get(ctx context.Context, in *stat.GetRequest, opts ...grpc.CallOption) (*stat.GetResponse, error)
	MultiGet(ctx context.Context, in *stat.MultiGetRequest, opts ...grpc.CallOption) (*stat.MultiGetResponse, error)
}

type statClient struct {
	cc *grpc.ClientConn
}

func NewStatClient(cc *grpc.ClientConn) StatClient {
	return &statClient{cc}
}

func (c *statClient) Init(ctx context.Context, in *stat.InitRequest, opts ...grpc.CallOption) (*stat.InitResponse, error) {
	out := new(stat.InitResponse)
	err := grpc.Invoke(ctx, "/statservice.Stat/Init", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *statClient) Clear(ctx context.Context, in *stat.ClearRequest, opts ...grpc.CallOption) (*stat.ClearResponse, error) {
	out := new(stat.ClearResponse)
	err := grpc.Invoke(ctx, "/statservice.Stat/Clear", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *statClient) Sync(ctx context.Context, in *stat.SyncRequest, opts ...grpc.CallOption) (*stat.SyncResponse, error) {
	out := new(stat.SyncResponse)
	err := grpc.Invoke(ctx, "/statservice.Stat/Sync", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *statClient) Get(ctx context.Context, in *stat.GetRequest, opts ...grpc.CallOption) (*stat.GetResponse, error) {
	out := new(stat.GetResponse)
	err := grpc.Invoke(ctx, "/statservice.Stat/Get", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *statClient) MultiGet(ctx context.Context, in *stat.MultiGetRequest, opts ...grpc.CallOption) (*stat.MultiGetResponse, error) {
	out := new(stat.MultiGetResponse)
	err := grpc.Invoke(ctx, "/statservice.Stat/MultiGet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for Stat service

type StatServer interface {
	Init(context.Context, *stat.InitRequest) (*stat.InitResponse, error)
	Clear(context.Context, *stat.ClearRequest) (*stat.ClearResponse, error)
	Sync(context.Context, *stat.SyncRequest) (*stat.SyncResponse, error)
	Get(context.Context, *stat.GetRequest) (*stat.GetResponse, error)
	MultiGet(context.Context, *stat.MultiGetRequest) (*stat.MultiGetResponse, error)
}

func RegisterStatServer(s *grpc.Server, srv StatServer) {
	s.RegisterService(&_Stat_serviceDesc, srv)
}

func _Stat_Init_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(stat.InitRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StatServer).Init(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/statservice.Stat/Init",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StatServer).Init(ctx, req.(*stat.InitRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Stat_Clear_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(stat.ClearRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StatServer).Clear(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/statservice.Stat/Clear",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StatServer).Clear(ctx, req.(*stat.ClearRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Stat_Sync_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(stat.SyncRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StatServer).Sync(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/statservice.Stat/Sync",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StatServer).Sync(ctx, req.(*stat.SyncRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Stat_Get_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(stat.GetRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StatServer).Get(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/statservice.Stat/Get",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StatServer).Get(ctx, req.(*stat.GetRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Stat_MultiGet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(stat.MultiGetRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StatServer).MultiGet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/statservice.Stat/MultiGet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StatServer).MultiGet(ctx, req.(*stat.MultiGetRequest))
	}
	return interceptor(ctx, in, info, handler)
}

var _Stat_serviceDesc = grpc.ServiceDesc{
	ServiceName: "statservice.Stat",
	HandlerType: (*StatServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "Init",
			Handler:    _Stat_Init_Handler,
		},
		{
			MethodName: "Clear",
			Handler:    _Stat_Clear_Handler,
		},
		{
			MethodName: "Sync",
			Handler:    _Stat_Sync_Handler,
		},
		{
			MethodName: "Get",
			Handler:    _Stat_Get_Handler,
		},
		{
			MethodName: "MultiGet",
			Handler:    _Stat_MultiGet_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: fileDescriptor0,
}

func init() { proto.RegisterFile("statservice.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 177 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x09, 0x6e, 0x88, 0x02, 0xff, 0xe2, 0x12, 0x2c, 0x2e, 0x49, 0x2c,
	0x29, 0x4e, 0x2d, 0x2a, 0xcb, 0x4c, 0x4e, 0xd5, 0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0xe2, 0x46,
	0x12, 0x92, 0xe2, 0x02, 0x71, 0x20, 0x12, 0x46, 0xed, 0x4c, 0x5c, 0x2c, 0xc1, 0x40, 0xae, 0x90,
	0x3e, 0x17, 0x8b, 0x67, 0x5e, 0x66, 0x89, 0x90, 0xa0, 0x1e, 0x58, 0x16, 0xc4, 0x0e, 0x4a, 0x2d,
	0x2c, 0x4d, 0x2d, 0x2e, 0x91, 0x12, 0x42, 0x16, 0x2a, 0x2e, 0xc8, 0xcf, 0x2b, 0x4e, 0x55, 0x62,
	0x10, 0x32, 0xe2, 0x62, 0x75, 0xce, 0x49, 0x4d, 0x2c, 0x12, 0x82, 0x4a, 0x83, 0x39, 0x30, 0x2d,
	0xc2, 0x28, 0x62, 0x70, 0x3d, 0x40, 0x4b, 0x82, 0x2b, 0xf3, 0x92, 0x61, 0x96, 0x80, 0xd8, 0x68,
	0x96, 0x40, 0x84, 0xe0, 0x1a, 0x74, 0xb8, 0x98, 0xdd, 0x53, 0x4b, 0x84, 0x04, 0x20, 0x92, 0x40,
	0x26, 0x4c, 0xb9, 0x20, 0x92, 0x08, 0x5c, 0xb5, 0x35, 0x17, 0x87, 0x6f, 0x69, 0x4e, 0x49, 0x26,
	0x48, 0x8b, 0x28, 0x44, 0x01, 0x8c, 0x0f, 0xd3, 0x27, 0x86, 0x2e, 0x0c, 0xd3, 0x9c, 0xc4, 0x06,
	0x0e, 0x10, 0x63, 0x40, 0x00, 0x00, 0x00, 0xff, 0xff, 0x42, 0x01, 0xb2, 0x95, 0x3e, 0x01, 0x00,
	0x00,
}
