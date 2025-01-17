
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "cn3block.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdfblock.h"
#include "sr3block.h"
#include "tx3block.h"
namespace mdf::detail {
class Cg3Block : public MdfBlock, public IChannelGroup {
 public:
  using Cn3List = std::vector<std::unique_ptr<Cn3Block>>;
  using Sr3List = std::vector<std::unique_ptr<Sr3Block>>;

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Name(const std::string& name) override;
  [[nodiscard]] std::string Name() const override;

  void Description(const std::string& description) override;
  [[nodiscard]] std::string Description() const override;

  [[nodiscard]] uint64_t NofSamples() const override;
  void NofSamples(uint64_t nof_samples) override;

  void RecordId(uint64_t record_id) override;
  [[nodiscard]] uint64_t RecordId() const override;

  [[nodiscard]] std::vector<IChannel*> Channels() const override;
  [[nodiscard]] IChannel* CreateChannel() override;

  [[nodiscard]] const IChannel* GetXChannel(
      const IChannel& reference) const override;

  [[nodiscard]] std::string Comment() const override;
  const MdfBlock* Find(int64_t index) const override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE* file) override;
  size_t Write(std::FILE* file) override;
  void ReadCnList(std::FILE* file);
  void ReadSrList(std::FILE* file);

  uint16_t RecordSize() const { return size_of_data_record_; }

  void AddCn3(std::unique_ptr<Cn3Block>& cn3);
  const Cn3List& Cn3() const { return cn_list_; }

  const Sr3List& Sr3() const { return sr_list_; }

  [[nodiscard]] std::vector<uint8_t>& SampleBuffer() const {
    return sample_buffer_;
  }
  size_t ReadDataRecord(std::FILE* file, const IDataGroup& notifier) const;
  void PrepareForWriting();
 private:
  uint16_t record_id_ = 0;
  uint16_t nof_channels_ = 0;
  uint16_t size_of_data_record_ = 0;
  uint32_t nof_records_ = 0;

  std::string comment_;
  Cn3List cn_list_;
  Sr3List sr_list_;


};
}  // namespace mdf::detail
