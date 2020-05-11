#include "SpeckleUnrealManager.h"

// Sets default values
ASpeckleUnrealManager::ASpeckleUnrealManager()
{
	//When the object is constructed, Get the HTTP module
	Http = &FHttpModule::Get();
}

// Called when the game starts or when spawned
void ASpeckleUnrealManager::BeginPlay()
{
	Super::BeginPlay();

	World = GetWorld();
	GetStream();
}

void ASpeckleUnrealManager::SetUpGetRequest(TSharedRef<IHttpRequest> Request)
{
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader("Authorization", AuthToken);
}

/*Http call*/
void ASpeckleUnrealManager::GetStream()
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();

	SetUpGetRequest(Request);

	Request->OnProcessRequestComplete().BindUObject(this, &ASpeckleUnrealManager::OnStreamResponseReceived);

	//This is the url on which to process the request
	Request->SetURL(ServerUrl + "api/streams/" + StreamID);

	Request->ProcessRequest();
}

/*Assigned function on successfull http call*/
void ASpeckleUnrealManager::OnStreamResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Red, "Stream Request failed");
		return;
	}

	//Create a pointer to hold the json serialized data
	TSharedPtr<FJsonObject> ResponseJsonObject;

	//Create a reader pointer to read the json data
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	//Deserialize the json data given Reader and the actual object to deserialize
	if (FJsonSerializer::Deserialize(Reader, ResponseJsonObject))
	{
		//Get the value of the json object by field name
		FString ResponseMessage = ResponseJsonObject->GetStringField("message");
		TSharedPtr<FJsonObject> Stream = ResponseJsonObject->GetObjectField("resource");
		FString StreamName = Stream->GetStringField("name");
		FString StreamDescription = Stream->GetStringField("description");

		//Output it to the engine
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, "Status: " + ResponseMessage);
		GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, "Name: " + StreamName);
		GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Green, "Description: " + StreamDescription);

		TArray<TSharedPtr<FJsonValue>> ObjectPlaceholderArray = Stream->GetArrayField("objects");

		GetStreamObjects(ObjectPlaceholderArray.Num());
	}
	else
	{
		// couldn't deserialize json from response
	}
}

void ASpeckleUnrealManager::GetStreamObjects(int32 objectCount)
{
	int32 RequestLimit = 20;

	for (size_t i = 0; i < objectCount; i += RequestLimit)
	{
		TSharedRef<IHttpRequest> Request = Http->CreateRequest();

		SetUpGetRequest(Request);

		Request->OnProcessRequestComplete().BindUObject(this, &ASpeckleUnrealManager::OnStreamObjectResponseReceived);

		//This is the url on which to process the request
		Request->SetURL(ServerUrl + "api/streams/" + StreamID + "/objects?limit=" + FString::FromInt(RequestLimit) + "&offset=" + FString::FromInt(i));

		Request->ProcessRequest();
	}
}

void ASpeckleUnrealManager::OnStreamObjectResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Red, "Object Request failed");
		return;
	}

	//Create a pointer to hold the json serialized data
	TSharedPtr<FJsonObject> ResponseJsonObject;

	//Create a reader pointer to read the json data
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	//Deserialize the json data given Reader and the actual object to deserialize
	if (FJsonSerializer::Deserialize(Reader, ResponseJsonObject))
	{
		//Get the value of the json object by field name
		TArray<TSharedPtr<FJsonValue>> StreamObjects = ResponseJsonObject->GetArrayField("resources");

		for (size_t i = 0; i < StreamObjects.Num(); i++)
		{
			TSharedPtr<FJsonObject> StreamObject = StreamObjects[i].Get()->AsObject();
			TArray<TSharedPtr<FJsonValue>> ObjectVertices = StreamObject->GetArrayField("vertices");

			AActor* MeshInstance = World->SpawnActor(MeshActor);

			/*for (size_t i = 0; i < length; i++)
			{

			}

			ObjectVertices[0].Get()->AsNumber();
			int32 vertCount = ObjectVertices.Num();
			float verticies[vertCount] = new float[];*/
		}



	}
	else
	{
		// couldn't deserialize json from response
	}
}