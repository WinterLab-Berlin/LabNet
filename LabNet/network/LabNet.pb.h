// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: LabNet.proto

#ifndef PROTOBUF_INCLUDED_LabNet_2eproto
#define PROTOBUF_INCLUDED_LabNet_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_LabNet_2eproto 

namespace protobuf_LabNet_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_LabNet_2eproto
namespace LabNetProt {
class PinId;
class PinIdDefaultTypeInternal;
extern PinIdDefaultTypeInternal _PinId_default_instance_;
}  // namespace LabNetProt
namespace google {
namespace protobuf {
template<> ::LabNetProt::PinId* Arena::CreateMaybeMessage<::LabNetProt::PinId>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace LabNetProt {

enum Interfaces {
  INTERFACE_NONE = 0,
  INTERFACE_IO_BOARD = 1,
  INTERFACE_RFID_BOARD = 2,
  INTERFACE_GPIO_WIRINGPI = 3,
  INTERFACE_SOUND = 4,
  INTERFACE_CHI_BIO = 5,
  INTERFACE_BLE_UART = 6,
  INTERFACE_UART0 = 100,
  INTERFACE_UART1 = 101,
  INTERFACE_UART2 = 102,
  INTERFACE_UART3 = 103,
  INTERFACE_UART4 = 104,
  Interfaces_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  Interfaces_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool Interfaces_IsValid(int value);
const Interfaces Interfaces_MIN = INTERFACE_NONE;
const Interfaces Interfaces_MAX = INTERFACE_UART4;
const int Interfaces_ARRAYSIZE = Interfaces_MAX + 1;

const ::google::protobuf::EnumDescriptor* Interfaces_descriptor();
inline const ::std::string& Interfaces_Name(Interfaces value) {
  return ::google::protobuf::internal::NameOfEnum(
    Interfaces_descriptor(), value);
}
inline bool Interfaces_Parse(
    const ::std::string& name, Interfaces* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Interfaces>(
    Interfaces_descriptor(), name, value);
}
enum Uarts {
  UART_NONE = 0,
  UART_PORT0 = 100,
  UART_PORT1 = 101,
  UART_PORT2 = 102,
  UART_PORT3 = 103,
  UART_PORT4 = 104,
  Uarts_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  Uarts_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool Uarts_IsValid(int value);
const Uarts Uarts_MIN = UART_NONE;
const Uarts Uarts_MAX = UART_PORT4;
const int Uarts_ARRAYSIZE = Uarts_MAX + 1;

const ::google::protobuf::EnumDescriptor* Uarts_descriptor();
inline const ::std::string& Uarts_Name(Uarts value) {
  return ::google::protobuf::internal::NameOfEnum(
    Uarts_descriptor(), value);
}
inline bool Uarts_Parse(
    const ::std::string& name, Uarts* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Uarts>(
    Uarts_descriptor(), name, value);
}
// ===================================================================

class PinId : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:LabNetProt.PinId) */ {
 public:
  PinId();
  virtual ~PinId();

  PinId(const PinId& from);

  inline PinId& operator=(const PinId& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  PinId(PinId&& from) noexcept
    : PinId() {
    *this = ::std::move(from);
  }

  inline PinId& operator=(PinId&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const PinId& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const PinId* internal_default_instance() {
    return reinterpret_cast<const PinId*>(
               &_PinId_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(PinId* other);
  friend void swap(PinId& a, PinId& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline PinId* New() const final {
    return CreateMaybeMessage<PinId>(NULL);
  }

  PinId* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<PinId>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const PinId& from);
  void MergeFrom(const PinId& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(PinId* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // .LabNetProt.Interfaces interface = 1;
  void clear_interface();
  static const int kInterfaceFieldNumber = 1;
  ::LabNetProt::Interfaces interface() const;
  void set_interface(::LabNetProt::Interfaces value);

  // uint32 pin = 2;
  void clear_pin();
  static const int kPinFieldNumber = 2;
  ::google::protobuf::uint32 pin() const;
  void set_pin(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:LabNetProt.PinId)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  int interface_;
  ::google::protobuf::uint32 pin_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_LabNet_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// PinId

// .LabNetProt.Interfaces interface = 1;
inline void PinId::clear_interface() {
  interface_ = 0;
}
inline ::LabNetProt::Interfaces PinId::interface() const {
  // @@protoc_insertion_point(field_get:LabNetProt.PinId.interface)
  return static_cast< ::LabNetProt::Interfaces >(interface_);
}
inline void PinId::set_interface(::LabNetProt::Interfaces value) {
  
  interface_ = value;
  // @@protoc_insertion_point(field_set:LabNetProt.PinId.interface)
}

// uint32 pin = 2;
inline void PinId::clear_pin() {
  pin_ = 0u;
}
inline ::google::protobuf::uint32 PinId::pin() const {
  // @@protoc_insertion_point(field_get:LabNetProt.PinId.pin)
  return pin_;
}
inline void PinId::set_pin(::google::protobuf::uint32 value) {
  
  pin_ = value;
  // @@protoc_insertion_point(field_set:LabNetProt.PinId.pin)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace LabNetProt

namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::LabNetProt::Interfaces> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::LabNetProt::Interfaces>() {
  return ::LabNetProt::Interfaces_descriptor();
}
template <> struct is_proto_enum< ::LabNetProt::Uarts> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::LabNetProt::Uarts>() {
  return ::LabNetProt::Uarts_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_LabNet_2eproto
