/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "PDInteractPreloader.h"

#define LOCTEXT_NAMESPACE "FPDInteractPreloaderModule"
DECLARE_LOG_CATEGORY_CLASS(PDLog_InteractPreLoader, Log, All);


constexpr const char* GSectionheader = "/Script/Engine.CollisionProfile"; 
constexpr FFileHelper::EHashOptions GDontHash = FFileHelper::EHashOptions::None;

#define ContainsEntry(SectionArray, Channel) \
SectionArray[Channel].GetValue().Contains("ECC_GameTraceChannel"#Channel"") 

#define BuildAttr(Channel, Response, InAppend) \
"(" \
"Channel=ECC_GameTraceChannel"#Channel "," \
"DefaultResponse="#Response "," \
"bTraceType=True," \
"bStaticObject=False," \
"Name=\""#InAppend "\""\
")"

void FPDInteractPreloaderModule::StartupModule()
{
	const FString ProjectFilePath = FPaths::ProjectConfigDir().Append(TEXT("DefaultEngine.ini"));
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString FileContent;
	
	FConfigFile DefaultEngineConfig;
	DefaultEngineConfig.Combine(ProjectFilePath);

	if (FileManager.FileExists(*ProjectFilePath) == false) { return; }

	const bool bFoundFile = FFileHelper::LoadFileToString(FileContent,*ProjectFilePath,GDontHash);
	if(bFoundFile == false) { return; }
	
	FConfigSection* CollisionSection = DefaultEngineConfig.Find(GSectionheader);
	if (CollisionSection != nullptr)
	{
		AttemptToWriteChannelResponses(CollisionSection);		
	}
}

#define ChannelResponseValue(SectionArray, Channel, Response, InAppend) \
ContainsEntry(SectionArray, Channel) ? "" : \
BuildAttr(Channel, Response, InAppend)

#define IfBegin_EntryAvailable(BuildString, Channel) \
BuildString = "ECC_GameTraceChannel" #Channel "was unused. Assiging as interact channel"; \
if (ContainsEntry(SectionValues, Channel) == false) \
{
	
#define IfEnd(BuildString, Channel) \
		CollisionSection->HandleAddCommand( \
			"+DefaultChannelResponses", \
			ChannelResponseValue(SectionValues, Channel, ECR_BLOCK, INTERACT_TRACE_ALT##Channel ), \
			false); \
		return; \
	} \
BuildString = "ECC_GameTraceChannel" #Channel "was NOT unused. Skipping and looking at next trace channel"; 


void FPDInteractPreloaderModule::AttemptToWriteChannelResponses(FConfigSection* CollisionSection)
{
	TArray<FConfigValue> SectionValues{};
	SectionValues.SetNum(19);
	int32 Idx = 0;
	for (const TTuple<FName, FConfigValue>& Pair : *CollisionSection)
	{
		const FName Name = Pair.Get<0>();
		const FConfigValue Value = Pair.Get<1>();
		if (Value.GetValue().Contains("ECC_GameTraceChannel" + FString::FromInt(Idx)))
		{
			SectionValues.Insert(Value, Idx);
			Idx++;
		}
	}

	// This will only add new commands if existing ones do not already exist
	FString BuildString;
	IfBegin_EntryAvailable(BuildString, 18)
		UE_LOG(PDLog_InteractPreLoader, Log, TEXT("%s"), *BuildString);
	IfEnd(BuildString, 18)
	UE_LOG(PDLog_InteractPreLoader, Warning, TEXT("%s"), *BuildString);

	IfBegin_EntryAvailable(BuildString, 17)
		UE_LOG(PDLog_InteractPreLoader, Log, TEXT("%s"), *BuildString);
	IfEnd(BuildString, 17)
	UE_LOG(PDLog_InteractPreLoader, Warning, TEXT("%s"), *BuildString);

	IfBegin_EntryAvailable(BuildString, 16)
		UE_LOG(PDLog_InteractPreLoader, Log, TEXT("%s"), *BuildString);
	IfEnd(BuildString, 16)	
	UE_LOG(PDLog_InteractPreLoader, Warning, TEXT("%s"), *BuildString);

	IfBegin_EntryAvailable(BuildString, 15)
		UE_LOG(PDLog_InteractPreLoader, Log, TEXT("%s"), *BuildString);
	IfEnd(BuildString, 15)
	UE_LOG(PDLog_InteractPreLoader, Warning, TEXT("%s"), *BuildString);

	IfBegin_EntryAvailable(BuildString, 14)
		UE_LOG(PDLog_InteractPreLoader, Log, TEXT("%s"), *BuildString);
	IfEnd(BuildString, 14)		
	UE_LOG(PDLog_InteractPreLoader, Warning, TEXT("%s"), *BuildString);

}

void FPDInteractPreloaderModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDInteractPreloaderModule, PDInteractPreloader);


/*
 * @copyright Permafrost Development (MIT license)
 * Authors: Ario Amin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
