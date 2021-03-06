// Code generated by protoc-gen-go.
// source: linkservice.proto
// DO NOT EDIT!

/*
Package linkservice is a generated protocol buffer package.

It is generated from these files:
	linkservice.proto

It has these top-level messages:
*/
package linkservice

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import link "github.com/ishidawataru/opennsl-server/client/proto/link"

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

// Client API for Link service

type LinkClient interface {
	Detach(ctx context.Context, in *link.DetachRequest, opts ...grpc.CallOption) (*link.DetachResponse, error)
	LinkscanEnableSet(ctx context.Context, in *link.LinkscanEnableSetRequest, opts ...grpc.CallOption) (*link.LinkscanEnableSetResponse, error)
	LinkscanEnableGet(ctx context.Context, in *link.LinkscanEnableGetRequest, opts ...grpc.CallOption) (*link.LinkscanEnableGetResponse, error)
	LinkscanModeSet(ctx context.Context, in *link.LinkscanModeSetRequest, opts ...grpc.CallOption) (*link.LinkscanModeSetResponse, error)
	LinkscanModeGet(ctx context.Context, in *link.LinkscanModeGetRequest, opts ...grpc.CallOption) (*link.LinkscanModeGetResponse, error)
	LinkscanModeSetPBM(ctx context.Context, in *link.LinkscanModeSetPBMRequest, opts ...grpc.CallOption) (*link.LinkscanModeSetPBMResponse, error)
	Monitor(ctx context.Context, in *link.MonitorRequest, opts ...grpc.CallOption) (Link_MonitorClient, error)
}

type linkClient struct {
	cc *grpc.ClientConn
}

func NewLinkClient(cc *grpc.ClientConn) LinkClient {
	return &linkClient{cc}
}

func (c *linkClient) Detach(ctx context.Context, in *link.DetachRequest, opts ...grpc.CallOption) (*link.DetachResponse, error) {
	out := new(link.DetachResponse)
	err := grpc.Invoke(ctx, "/linkservice.Link/Detach", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *linkClient) LinkscanEnableSet(ctx context.Context, in *link.LinkscanEnableSetRequest, opts ...grpc.CallOption) (*link.LinkscanEnableSetResponse, error) {
	out := new(link.LinkscanEnableSetResponse)
	err := grpc.Invoke(ctx, "/linkservice.Link/LinkscanEnableSet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *linkClient) LinkscanEnableGet(ctx context.Context, in *link.LinkscanEnableGetRequest, opts ...grpc.CallOption) (*link.LinkscanEnableGetResponse, error) {
	out := new(link.LinkscanEnableGetResponse)
	err := grpc.Invoke(ctx, "/linkservice.Link/LinkscanEnableGet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *linkClient) LinkscanModeSet(ctx context.Context, in *link.LinkscanModeSetRequest, opts ...grpc.CallOption) (*link.LinkscanModeSetResponse, error) {
	out := new(link.LinkscanModeSetResponse)
	err := grpc.Invoke(ctx, "/linkservice.Link/LinkscanModeSet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *linkClient) LinkscanModeGet(ctx context.Context, in *link.LinkscanModeGetRequest, opts ...grpc.CallOption) (*link.LinkscanModeGetResponse, error) {
	out := new(link.LinkscanModeGetResponse)
	err := grpc.Invoke(ctx, "/linkservice.Link/LinkscanModeGet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *linkClient) LinkscanModeSetPBM(ctx context.Context, in *link.LinkscanModeSetPBMRequest, opts ...grpc.CallOption) (*link.LinkscanModeSetPBMResponse, error) {
	out := new(link.LinkscanModeSetPBMResponse)
	err := grpc.Invoke(ctx, "/linkservice.Link/LinkscanModeSetPBM", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *linkClient) Monitor(ctx context.Context, in *link.MonitorRequest, opts ...grpc.CallOption) (Link_MonitorClient, error) {
	stream, err := grpc.NewClientStream(ctx, &_Link_serviceDesc.Streams[0], c.cc, "/linkservice.Link/Monitor", opts...)
	if err != nil {
		return nil, err
	}
	x := &linkMonitorClient{stream}
	if err := x.ClientStream.SendMsg(in); err != nil {
		return nil, err
	}
	if err := x.ClientStream.CloseSend(); err != nil {
		return nil, err
	}
	return x, nil
}

type Link_MonitorClient interface {
	Recv() (*link.MonitorResponse, error)
	grpc.ClientStream
}

type linkMonitorClient struct {
	grpc.ClientStream
}

func (x *linkMonitorClient) Recv() (*link.MonitorResponse, error) {
	m := new(link.MonitorResponse)
	if err := x.ClientStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

// Server API for Link service

type LinkServer interface {
	Detach(context.Context, *link.DetachRequest) (*link.DetachResponse, error)
	LinkscanEnableSet(context.Context, *link.LinkscanEnableSetRequest) (*link.LinkscanEnableSetResponse, error)
	LinkscanEnableGet(context.Context, *link.LinkscanEnableGetRequest) (*link.LinkscanEnableGetResponse, error)
	LinkscanModeSet(context.Context, *link.LinkscanModeSetRequest) (*link.LinkscanModeSetResponse, error)
	LinkscanModeGet(context.Context, *link.LinkscanModeGetRequest) (*link.LinkscanModeGetResponse, error)
	LinkscanModeSetPBM(context.Context, *link.LinkscanModeSetPBMRequest) (*link.LinkscanModeSetPBMResponse, error)
	Monitor(*link.MonitorRequest, Link_MonitorServer) error
}

func RegisterLinkServer(s *grpc.Server, srv LinkServer) {
	s.RegisterService(&_Link_serviceDesc, srv)
}

func _Link_Detach_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(link.DetachRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(LinkServer).Detach(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/linkservice.Link/Detach",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(LinkServer).Detach(ctx, req.(*link.DetachRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Link_LinkscanEnableSet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(link.LinkscanEnableSetRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(LinkServer).LinkscanEnableSet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/linkservice.Link/LinkscanEnableSet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(LinkServer).LinkscanEnableSet(ctx, req.(*link.LinkscanEnableSetRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Link_LinkscanEnableGet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(link.LinkscanEnableGetRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(LinkServer).LinkscanEnableGet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/linkservice.Link/LinkscanEnableGet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(LinkServer).LinkscanEnableGet(ctx, req.(*link.LinkscanEnableGetRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Link_LinkscanModeSet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(link.LinkscanModeSetRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(LinkServer).LinkscanModeSet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/linkservice.Link/LinkscanModeSet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(LinkServer).LinkscanModeSet(ctx, req.(*link.LinkscanModeSetRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Link_LinkscanModeGet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(link.LinkscanModeGetRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(LinkServer).LinkscanModeGet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/linkservice.Link/LinkscanModeGet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(LinkServer).LinkscanModeGet(ctx, req.(*link.LinkscanModeGetRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Link_LinkscanModeSetPBM_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(link.LinkscanModeSetPBMRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(LinkServer).LinkscanModeSetPBM(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/linkservice.Link/LinkscanModeSetPBM",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(LinkServer).LinkscanModeSetPBM(ctx, req.(*link.LinkscanModeSetPBMRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Link_Monitor_Handler(srv interface{}, stream grpc.ServerStream) error {
	m := new(link.MonitorRequest)
	if err := stream.RecvMsg(m); err != nil {
		return err
	}
	return srv.(LinkServer).Monitor(m, &linkMonitorServer{stream})
}

type Link_MonitorServer interface {
	Send(*link.MonitorResponse) error
	grpc.ServerStream
}

type linkMonitorServer struct {
	grpc.ServerStream
}

func (x *linkMonitorServer) Send(m *link.MonitorResponse) error {
	return x.ServerStream.SendMsg(m)
}

var _Link_serviceDesc = grpc.ServiceDesc{
	ServiceName: "linkservice.Link",
	HandlerType: (*LinkServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "Detach",
			Handler:    _Link_Detach_Handler,
		},
		{
			MethodName: "LinkscanEnableSet",
			Handler:    _Link_LinkscanEnableSet_Handler,
		},
		{
			MethodName: "LinkscanEnableGet",
			Handler:    _Link_LinkscanEnableGet_Handler,
		},
		{
			MethodName: "LinkscanModeSet",
			Handler:    _Link_LinkscanModeSet_Handler,
		},
		{
			MethodName: "LinkscanModeGet",
			Handler:    _Link_LinkscanModeGet_Handler,
		},
		{
			MethodName: "LinkscanModeSetPBM",
			Handler:    _Link_LinkscanModeSetPBM_Handler,
		},
	},
	Streams: []grpc.StreamDesc{
		{
			StreamName:    "Monitor",
			Handler:       _Link_Monitor_Handler,
			ServerStreams: true,
		},
	},
	Metadata: fileDescriptor0,
}

func init() { proto.RegisterFile("linkservice.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 223 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x09, 0x6e, 0x88, 0x02, 0xff, 0xe2, 0x12, 0xcc, 0xc9, 0xcc, 0xcb,
	0x2e, 0x4e, 0x2d, 0x2a, 0xcb, 0x4c, 0x4e, 0xd5, 0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0xe2, 0x46,
	0x12, 0x92, 0xe2, 0x02, 0x71, 0x20, 0x12, 0x46, 0x2b, 0x58, 0xb8, 0x58, 0x7c, 0x80, 0x5c, 0x21,
	0x53, 0x2e, 0x36, 0x97, 0xd4, 0x92, 0xc4, 0xe4, 0x0c, 0x21, 0x61, 0x3d, 0xb0, 0x3c, 0x84, 0x17,
	0x94, 0x5a, 0x58, 0x9a, 0x5a, 0x5c, 0x22, 0x25, 0x82, 0x2a, 0x58, 0x5c, 0x90, 0x9f, 0x57, 0x9c,
	0xaa, 0xc4, 0x20, 0x14, 0xc6, 0x25, 0x08, 0xd2, 0x5e, 0x9c, 0x9c, 0x98, 0xe7, 0x9a, 0x97, 0x98,
	0x94, 0x93, 0x1a, 0x9c, 0x5a, 0x22, 0x24, 0x07, 0x51, 0x8c, 0x21, 0x01, 0x33, 0x4c, 0x1e, 0xa7,
	0x3c, 0x6e, 0x73, 0xdd, 0x71, 0x99, 0xeb, 0x4e, 0xc0, 0x5c, 0x77, 0x14, 0x73, 0x03, 0xb8, 0xf8,
	0x61, 0xd2, 0xbe, 0xf9, 0x29, 0x60, 0xd7, 0xca, 0xa0, 0xea, 0x82, 0x0a, 0xc3, 0xcc, 0x94, 0xc5,
	0x21, 0x8b, 0xcb, 0x44, 0x77, 0xec, 0x26, 0xba, 0xe3, 0x35, 0x11, 0xd5, 0x8d, 0x91, 0x5c, 0x42,
	0x68, 0xd6, 0x05, 0x38, 0xf9, 0x0a, 0xc9, 0x63, 0x75, 0x08, 0x50, 0x06, 0x66, 0xae, 0x02, 0x6e,
	0x05, 0x70, 0xa3, 0xad, 0xb8, 0xd8, 0x7d, 0xf3, 0xf3, 0x32, 0x4b, 0xf2, 0x8b, 0x84, 0xa0, 0x31,
	0x0a, 0xe5, 0xc2, 0x0c, 0x11, 0x45, 0x13, 0x85, 0xe9, 0x34, 0x60, 0x4c, 0x62, 0x03, 0xa7, 0x18,
	0x63, 0x40, 0x00, 0x00, 0x00, 0xff, 0xff, 0x60, 0x4c, 0x79, 0x49, 0x5f, 0x02, 0x00, 0x00,
}
