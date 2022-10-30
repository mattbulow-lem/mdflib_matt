﻿/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>

#include "MdfFile.h"

using namespace msclr::interop;

namespace MdfLibrary {

array<MdfAttachment^>^ MdfFile::Attachments::get() {
  mdf::AttachmentList list;
  if (mdf_file_ != nullptr) {
    mdf_file_->Attachments(list);
  }
  array<MdfAttachment^>^ temp_list =
    gcnew array<MdfAttachment^>(static_cast<int>(list.size()));
  for (int index = 0; index < list.size(); ++index) {
    auto* temp = const_cast<mdf::IAttachment*>(list[index]);
    temp_list[static_cast<int>(index)] = gcnew MdfAttachment(temp);
  }
  return temp_list;
}

array<MdfDataGroup^>^ MdfFile::DataGroups::get() {
   mdf::DataGroupList list;
   if (mdf_file_ != nullptr) {
     mdf_file_->DataGroups(list);
   }
   array<MdfDataGroup^>^ temp_list =
     gcnew array<MdfDataGroup^>(static_cast<int>(list.size()));
   for (int index = 0; index < list.size(); ++index) {
     auto* temp = const_cast<mdf::IDataGroup*>(list[index]);
     temp_list[static_cast<int>(index)] = gcnew MdfDataGroup(temp);
   }
   return temp_list;
}

String^ MdfFile::Name::get() {
  const auto* temp = mdf_file_ != nullptr ?
    mdf_file_->Name().c_str() : "";
  return  gcnew String(temp);
}

void MdfFile::Name::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
    std::string() : marshal_as<std::string>(name);
  if (mdf_file_ != nullptr) {
    mdf_file_->Name(temp);
  }
}

String^ MdfFile::Filename::get() {
  const auto* temp = mdf_file_ != nullptr ?
    mdf_file_->FileName().c_str() : "";
  return  gcnew String(temp);
}

void MdfFile::Filename::set(String^ filename) {
  const auto temp = String::IsNullOrEmpty(filename) ?
    std::string() : marshal_as<std::string>(filename);
  if (mdf_file_ != nullptr) {
    mdf_file_->FileName(temp);
  }  
}

String^ MdfFile::Version::get() {
  const auto temp = mdf_file_ != nullptr ?
      mdf_file_->Version() : std::string();
  return  gcnew String(temp.c_str());
}

int MdfFile::MainVersion::get() {
  return  mdf_file_ != nullptr ? mdf_file_->MainVersion() : 0;
}

int MdfFile::MinorVersion::get() {
  return  mdf_file_ != nullptr ? mdf_file_->MinorVersion() : 0;  
}
void MdfFile::MinorVersion::set(int minor) {
  if (mdf_file_ != nullptr) {
    mdf_file_->MinorVersion(minor);
  }
}

String^ MdfFile::ProgramId::get() {
  const auto temp = mdf_file_ != nullptr ?
      mdf_file_->ProgramId() : std::string();
  return  gcnew String(temp.c_str());  
}

void MdfFile::ProgramId::set(String^ program_id) {
  const auto temp = String::IsNullOrEmpty(program_id) ?
    std::string() : marshal_as<std::string>(program_id);
  if (mdf_file_ != nullptr) {
    mdf_file_->ProgramId(temp);
  }  
}

bool MdfFile::Finalized::get() {
  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  return mdf_file_ != nullptr ?
    mdf_file_->IsFinalized(standard_flags, custom_flags) : false;
}

MdfHeader^ MdfFile::Header::get() {
  auto* header = mdf_file_ != nullptr ? mdf_file_->Header() : nullptr;
  return gcnew MdfHeader(header);
}

MdfAttachment^ MdfFile::CreateAttachment() {
  auto* attachment = mdf_file_ != nullptr ?
    mdf_file_->CreateAttachment() : nullptr;
  return gcnew MdfAttachment(attachment);
}

MdfDataGroup^ MdfFile::CreateDataGroup() {
  auto* data_group = mdf_file_ != nullptr ?
    mdf_file_->CreateDataGroup() : nullptr;
  return gcnew MdfDataGroup(data_group);  
}

bool MdfFile::IsMdf4() {
  return mdf_file_ != nullptr ? mdf_file_->IsMdf4() : false;
};


MdfFile::MdfFile(mdf::MdfFile* mdf_file)
  : mdf_file_( mdf_file) {;
}
  
} // end namespace