/* mdflib_writer_matt.cpp : This file contains the 'main' function. Program execution begins and ends there.
* In order for this to build in visual studio comunity 2022, you must install some libraries with vcpkg:
* ZLIB Library
* EXPAT Library
*
* Then build using 'Release' settings, 'Debug' settings causes lots of errors
*
*/


// Known differences between file created from this script and known 'good' file
// ID Block: id_ver: 4.20 instead of 4.10
// Header Block: hd_time_flags: 0 instead of 2 [offsets valid]
// File History Block: time offsets are 0 instead of -7 for mountain time
// Channel Group Block:
//      no source info block
//      flags is 2 [Bus Event instead] of 6 [BUS Event | Plain BUS Event]
// Channel 'Timestamp': cn_precision is 0 instead of 3

#include <iostream>

// this  function is only needed for writing a test MDF file
#include <chrono>
uint64_t epoch_time() {
	// get time in nano-seconds since Jan 1,1970, or epoch time
	auto now = std::chrono::system_clock::now();
	auto since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
	uint64_t epoch_ns = since_epoch.count();
	return epoch_ns;
}


#include "main.h"

#include <mdf/mdffile.h>
#include <mdf/mdffactory.h>
#include <mdf/mdfwriter.h>
#include <mdf/iblock.h>
#include <mdf/idatagroup.h>
#include <mdf/ichannelgroup.h>
#include <mdf/ichannel.h>
#include <mdf/ichannelhierarchy.h>
#include <mdf/iheader.h>
#include <mdf/ifilehistory.h>
#include <mdf/isourceinformation.h>

using namespace mdf;

int main()
{
	std::cout << "Hello World!\n";

	/*
	The MDF writer purpose is to create MDF files. It simplifies the writing into some steps.
	The first is to create type of writer. This is done by using the MdfFactory::CreateMdfWriter() function.
	The next step is to call the Init() function with a filename with valid path. The Init function will create the actual MDF file object and check if it exist or not. If it exist, it reads in the existing file content so it can be appended.
	The next step is to prepare the file for a new measurement. This is done by creating the DG/CG/CN/CC blocks that defines the measurement. Note that it is the last DG block that is the target.
	Next step is to call the InitMeasurement() function. This create a thread that handle the queue of samples. The function also write the configuration to the file and closes it.
	The user shall know starts adding samples to the queue by first setting the current channel value to each channel and then call the SaveSample() function for each channel group (CG). Note that no samples are saved to the file yet. The max queue size is set to the pre-trig time, see PreTrigTime().
	At some point the user shall call the StartMeasurement() function. The sample queue is now saved onto the file. The save rate is actually dependent if CompressData() is enabled or not. If compression is used the data is saved at the 4MB size or a 10 minute max. If not using compression, the samples are saved each 10 second.
	The user shall now call StopMeasurement() which flush out the remaining sample queue. After stopping the queue, the user may add some extra block as event (EV) and attachment (AT) blocks.
	The FinalizeMeasurement() function. Stops the thread and write all unwritten blocks to the file.
	*/

	const char* filename = "test2.mf4";

	// Attempt to delete the file
	if (std::remove(filename) == 0) {
		// Deletion succeeded
		std::cout << "File '" << filename << "' deleted successfully." << std::endl;
	}
	else {
		// Deletion failed
		std::cout << "Error deleting file '" << filename << "'." << std::endl;
	}


	std::unique_ptr<MdfWriter> writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);

	bool flag = writer->Init(filename);
	std::cout << "Did Init Work? " << flag << std::endl;



	// Header()
	IHeader* header = writer->Header();
	header->Author("Matthew Bulow");
	header->Department("Home Alone");
	header->Description("Testing Sample");
	header->Project("Mdf4WriteCanData");
	header->StartTime(epoch_time());
	header->Subject("TelematicsLogger");
	// Make Channel Higharchy Block, what is this used for?
	//IChannelHierarchy* hierarchy = header->CreateChannelHierarchy();
	//hierarchy->Name("Test hierarchy");
	//hierarchy->Type(ChType::Structure);

	// file history block (FH)
	IFileHistory* history = header->CreateFileHistory();
	//history->Time(time_stamp);
	history->Description("Initial stuff");
	history->ToolName("Unit Test");
	history->ToolVendor("ACME");
	history->ToolVersion("2.3");
	history->UserName("Ducky");


	// Make DataGroup
	IDataGroup* dg = writer->CreateDataGroup();

	// Make Channel Group inside Data Group
	IChannelGroup* cg = dg->CreateChannelGroup();
	//  set channel group properties
	cg->Name("CAN");
	cg->Flags(CgFlag::BusEvent | CgFlag::PlainBusEvent);
	cg->PathSeparator(char16_t('.'));

	ISourceInformation* si = cg->CreateSourceInformation();
	si->Name("CAN");
	si->Path("CAN_DataFrame");
	si->Description("Trying to get this working in CANalyzer, but need SubChannels");
	si->Type(SourceType::Bus);
	si->Bus(BusType::Can);

	// Make Channels inside Channel Group
	IChannel* cn;

	// 0: Timestamp
	cn = cg->CreateChannel();
	cn->Name("Timestamp");
	cn->Unit("s");
	//cn->Flags(0);
	cn->Type(ChannelType::Master);
	cn->Sync(ChannelSyncType::Time);
	cn->DataType(ChannelDataType::FloatLe);
	cn->DataBytes(8);

	// CAN_DataFrame Channel and sub channels
	//		  Channel = CAN_DataFrame -> startByte  8; 17 bytes,   (Note this is a 'Structure Channel'
	//			  Member = BusChannel -> startByte  8;  1 byte,    (Note startbit is from start of ChannelGroup, not from start of Channel)
	//			  Member = ID		  -> startByte  9;  4 bytes,   CAN ID
	//			  Member = IDE		  -> startByte 13;  1 byte,    Is the ID Extended (0 = standard, 1 = extended)
	//			  Member = DLC		  -> startByte 14;  1 byte,    number of bytes in the CAN message (possible values for non CAN FD -> 0-8 bytes)
	//			  Member = DataLength -> startByte 15;  1 byte,    actual data length (same as DLC for non CAN FD frames)
	//			  Member = DataBytes  -> startByte 16;  8 byte,    actual raw hex data of CAN frame/message
	//			  Member = Dir		  -> startByte 24;  1 byte,    direction (recieved, transmitted)

	// 1: CAN_DataFrame channel
	cn = cg->CreateChannel();
	cn->Name("CAN_DataFrame");
	cn->Flags(CnFlag::BusEvent);
	cn->DataType(ChannelDataType::ByteArray);
	cn->DataBytes(17);

	// 2: BusChannel
	IChannel* cn_cn = cn->CreateChannel();
	cn_cn->Name("CAN_DataFrame.BusChannel");
	cn_cn->Flags(CnFlag::BusEvent);
	cn_cn->DataBytes(1);
	// 3: ID
	cn_cn = cn->CreateChannel();
	cn_cn->Name("CAN_DataFrame.ID");
	cn_cn->Flags(CnFlag::BusEvent);
	cn_cn->DataBytes(4);
	// 4: IDE
	cn_cn = cn->CreateChannel();
	cn_cn->Name("CAN_DataFrame.IDE");
	cn_cn->Flags(CnFlag::BusEvent);
	cn_cn->DataBytes(1);
	// 5: DLC
	cn_cn = cn->CreateChannel();
	cn_cn->Name("CAN_DataFrame.DLC");
	cn_cn->Flags(CnFlag::BusEvent);
	cn_cn->DataBytes(1);
	// 6: DataLength
	cn_cn = cn->CreateChannel();
	cn_cn->Name("CAN_DataFrame.DataLength");
	cn_cn->Flags(CnFlag::BusEvent);
	cn_cn->DataBytes(1);
	// 7: DataBytes
	cn_cn = cn->CreateChannel();
	cn_cn->Name("CAN_DataFrame.DataBytes");
	cn_cn->Flags(CnFlag::BusEvent);
	cn_cn->DataBytes(8);
	cn_cn->DataType(ChannelDataType::ByteArray);
	// 8: Dir
	cn_cn = cn->CreateChannel();
	cn_cn->Name("CAN_DataFrame.Dir");
	cn_cn->Flags(CnFlag::BusEvent);
	cn_cn->DataBytes(1);
	//// 9: EDL
	//cn = cg->CreateChannel();
	//cn->Name("CAN_DataFrame.EDL");
	//cn->Flags(CnFlag::BusEvent);
	//cn->DataBytes(1);
	//// 10: ESI
	//cn = cg->CreateChannel();
	//cn->Name("CAN_DataFrame.ESI");
	//cn->Flags(CnFlag::BusEvent);
	//cn->DataBytes(1);
	//// 11: BRS
	//cn = cg->CreateChannel();
	//cn->Name("CAN_DataFrame.BRS");
	//cn->Flags(CnFlag::BusEvent);
	//cn->DataBytes(1);


	// Add data to channels
	// 
	// This create a thread that handle the queue of samples. The function also write the configuration to the file and closes it.
	flag = writer->InitMeasurement();
	std::cout << "Did Init Measurement Work? " << flag << std::endl;	

	auto cg_cn = cg->Channels();
	writer->StartMeasurement(epoch_time());
	for (uint64_t n = 0; n < 1000; n++)
	{
		std::cout << ".";
		// set channel values
		cg_cn[0]->SetChannelValue(0.01 * static_cast<double>(n));
		// shouldnt need to set channel [1] 'CAN_DataFrame' due to sub channels
		cg_cn[2]->SetChannelValue(1);
		cg_cn[3]->SetChannelValue(0x7FF);
		cg_cn[4]->SetChannelValue(0);
		cg_cn[5]->SetChannelValue(8);
		cg_cn[6]->SetChannelValue(8);
		std::vector<uint8_t> myVector = { 1, 2, 3, 4, 5, 6, 7, 8 };
		for (auto& element : myVector) { element *= n; }
		cg_cn[7]->SetChannelValue(myVector);
		cg_cn[8]->SetChannelValue(0);
	//	cg_cn[toInt(CanChName::EDL)]->SetChannelValue(0);
	//	cg_cn[toInt(CanChName::ESI)]->SetChannelValue(0);
	//	cg_cn[toInt(CanChName::BRS)]->SetChannelValue(0);

		//save channel values
		writer->SaveSample(*cg, epoch_time());
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	writer->StopMeasurement(epoch_time());

	flag = writer->FinalizeMeasurement();
	std::cout << std::endl << "Did Finalize Work? " << flag << std::endl;




	return 0;
}

