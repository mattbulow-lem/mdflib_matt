/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

#include "mdf/mdffile.h"
#include "samplerecord.h"

namespace mdf {

class IChannelGroup;
class IChannel;
class IChannelConversion;
class IDataGroup;

class MdfWriter {
 public:
  MdfWriter() = default;
  virtual ~MdfWriter();

  MdfWriter(const MdfWriter& writer) = delete;
  MdfWriter& operator=(const MdfWriter& writer) = delete;

  std::string Name() const;

  bool Init(const std::string& filename);

  [[nodiscard]] bool IsFileNew() const {
    return write_state_ == WriteState::Create;
  }

  /** \brief Sets the pre-trig time (s) of the writer.
   *
   * Sets the pre-trig time (note seconds). This defines the maximum number of
   * samples the writer holds internally, before the measurement is started.
   * @param pre_trig_time Pre-trig time in seconds.
   */
  void PreTrigTime(double pre_trig_time);
  [[nodiscard]] double PreTrigTime() const;

  [[nodiscard]] uint64_t StartTime() const { return start_time_; }
  [[nodiscard]] uint64_t StopTime() const { return stop_time_; }

  /** \brief Returns the MDF file interface.
   *
   * Returns the MDF file interface. The user may change the file version and
   * the finalize status of the file. By default is the MDF 4.2 or 3.2 version
   * of the file used.
   * @return Pointer to the MDF file interface.
   */
  MdfFile* GetFile() { return mdf_file_.get(); }

  IHeader* Header() const;
  IDataGroup* CreateDataGroup();

  [[nodiscard]] static IChannelGroup* CreateChannelGroup(IDataGroup* parent);

  [[nodiscard]] static IChannel* CreateChannel(IChannelGroup* parent);

  virtual IChannelConversion* CreateChannelConversion(IChannel* parent) = 0;

  virtual bool InitMeasurement();
  void SaveSample(IChannelGroup& group, uint64_t time);
  void StartMeasurement(uint64_t start_time);
  void StopMeasurement(uint64_t stop_time);
  bool FinalizeMeasurement();

  void CompressData(bool compress) {compress_data_ = compress;}
  [[nodiscard]] bool CompressData() const { return compress_data_;}

 protected:
  enum class WriteState : uint8_t {
    Create,     ///< Only at first measurement
    Init,       ///< Start work thread and start collecting samples
    StartMeas,  ///< Start saving samples to file
    StopMeas,   ///< Stop saving samples. OK to
    Finalize    ///< OK to add new DG and CG blocks
  };
  std::atomic<WriteState> write_state_ =
      WriteState::Create;  ///< Keeps track of the worker thread state.

  std::unique_ptr<MdfFile> mdf_file_;  ///< Holds the actual file object.
  std::string filename_;  ///< Full name of file with path and extension.

  std::atomic<uint64_t> pre_trig_time_ = 0;  ///< Nanoseconds difference.
  std::atomic<uint64_t> start_time_ = 0;     ///< Nanoseconds since 1970.
  std::atomic<uint64_t> stop_time_ = 0;      ///< Nanoseconds since 1970.

  std::thread work_thread_;
  std::atomic_bool stop_thread_ = false;
  std::mutex locker_;
  std::condition_variable sample_event_;

  using SampleQueue = std::deque<SampleRecord>;
  SampleQueue sample_queue_;

  virtual void CreateMdfFile() = 0;
  virtual bool PrepareForWriting() = 0;
  virtual void SetDataPosition(std::FILE* file);
  virtual bool WriteSignalData(std::FILE* file);

  void StopWorkThread();
  void WorkThread();
  void TrimQueue();
  virtual void SaveQueue(std::unique_lock<std::mutex>& lock);
  virtual void CleanQueue(std::unique_lock<std::mutex>& lock);

  void IncrementNofSamples(uint64_t record_id) const;
  virtual void SetLastPosition(std::FILE* file) = 0;

 private:
  bool compress_data_ = false;
};

}  // namespace mdf