# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: LabNet.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='LabNet.proto',
  package='LabNetProt',
  syntax='proto3',
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_pb=b'\n\x0cLabNet.proto\x12\nLabNetProt\"?\n\x05PinId\x12)\n\tinterface\x18\x01 \x01(\x0e\x32\x16.LabNetProt.Interfaces\x12\x0b\n\x03pin\x18\x02 \x01(\r*\xed\x01\n\nInterfaces\x12\x12\n\x0eINTERFACE_NONE\x10\x00\x12\x16\n\x12INTERFACE_IO_BOARD\x10\x01\x12\x18\n\x14INTERFACE_RFID_BOARD\x10\x02\x12\x1b\n\x17INTERFACE_GPIO_WIRINGPI\x10\x03\x12\x13\n\x0fINTERFACE_SOUND\x10\x04\x12\x13\n\x0fINTERFACE_UART0\x10\x64\x12\x13\n\x0fINTERFACE_UART1\x10\x65\x12\x13\n\x0fINTERFACE_UART2\x10\x66\x12\x13\n\x0fINTERFACE_UART3\x10g\x12\x13\n\x0fINTERFACE_UART4\x10h*f\n\x05Uarts\x12\r\n\tUART_NONE\x10\x00\x12\x0e\n\nUART_PORT0\x10\x64\x12\x0e\n\nUART_PORT1\x10\x65\x12\x0e\n\nUART_PORT2\x10\x66\x12\x0e\n\nUART_PORT3\x10g\x12\x0e\n\nUART_PORT4\x10hb\x06proto3'
)

_INTERFACES = _descriptor.EnumDescriptor(
  name='Interfaces',
  full_name='LabNetProt.Interfaces',
  filename=None,
  file=DESCRIPTOR,
  create_key=_descriptor._internal_create_key,
  values=[
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_NONE', index=0, number=0,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_IO_BOARD', index=1, number=1,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_RFID_BOARD', index=2, number=2,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_GPIO_WIRINGPI', index=3, number=3,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_SOUND', index=4, number=4,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_UART0', index=5, number=100,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_UART1', index=6, number=101,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_UART2', index=7, number=102,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_UART3', index=8, number=103,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='INTERFACE_UART4', index=9, number=104,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=94,
  serialized_end=331,
)
_sym_db.RegisterEnumDescriptor(_INTERFACES)

Interfaces = enum_type_wrapper.EnumTypeWrapper(_INTERFACES)
_UARTS = _descriptor.EnumDescriptor(
  name='Uarts',
  full_name='LabNetProt.Uarts',
  filename=None,
  file=DESCRIPTOR,
  create_key=_descriptor._internal_create_key,
  values=[
    _descriptor.EnumValueDescriptor(
      name='UART_NONE', index=0, number=0,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='UART_PORT0', index=1, number=100,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='UART_PORT1', index=2, number=101,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='UART_PORT2', index=3, number=102,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='UART_PORT3', index=4, number=103,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='UART_PORT4', index=5, number=104,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=333,
  serialized_end=435,
)
_sym_db.RegisterEnumDescriptor(_UARTS)

Uarts = enum_type_wrapper.EnumTypeWrapper(_UARTS)
INTERFACE_NONE = 0
INTERFACE_IO_BOARD = 1
INTERFACE_RFID_BOARD = 2
INTERFACE_GPIO_WIRINGPI = 3
INTERFACE_SOUND = 4
INTERFACE_UART0 = 100
INTERFACE_UART1 = 101
INTERFACE_UART2 = 102
INTERFACE_UART3 = 103
INTERFACE_UART4 = 104
UART_NONE = 0
UART_PORT0 = 100
UART_PORT1 = 101
UART_PORT2 = 102
UART_PORT3 = 103
UART_PORT4 = 104



_PINID = _descriptor.Descriptor(
  name='PinId',
  full_name='LabNetProt.PinId',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='interface', full_name='LabNetProt.PinId.interface', index=0,
      number=1, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='pin', full_name='LabNetProt.PinId.pin', index=1,
      number=2, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=28,
  serialized_end=91,
)

_PINID.fields_by_name['interface'].enum_type = _INTERFACES
DESCRIPTOR.message_types_by_name['PinId'] = _PINID
DESCRIPTOR.enum_types_by_name['Interfaces'] = _INTERFACES
DESCRIPTOR.enum_types_by_name['Uarts'] = _UARTS
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

PinId = _reflection.GeneratedProtocolMessageType('PinId', (_message.Message,), {
  'DESCRIPTOR' : _PINID,
  '__module__' : 'LabNet_pb2'
  # @@protoc_insertion_point(class_scope:LabNetProt.PinId)
  })
_sym_db.RegisterMessage(PinId)


# @@protoc_insertion_point(module_scope)