
/*
#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>
#include<stdio.h>
using namespace fbxsdk;

int main()
{
    FbxManager* lSdkManager = FbxManager::Create();

    // Create an IOSettings object.
    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    // ... Configure the FbxIOSettings object ...

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

    // Declare the path and filename of the file containing the scene.
    // In this case, we are assuming the file is in the same directory as the executable.
    const char* lFilename = "D:\\Inception\\Content\\Models\\HornetNormalAttackOne1.FBX";

    // Initialize the importer.
    bool lImportStatus = lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings());


    if (!lImportStatus) {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }

    // Create a new scene so it can be populated by the imported file.
    FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

    // Import the contents of the file into the scene.
    lImporter->Import(lScene);

    // The file has been imported; we can get rid of the importer.
    lImporter->Destroy();

    FbxAxisSystem s(FbxAxisSystem::EPreDefinedAxisSystem::eOpenGL);
    s.ConvertScene(lScene);



    // ... Configure the FbxIOSettings object here ...

    // Create an exporter.
    FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

    // Declare the path and filename of the file to which the scene will be exported.
    // In this case, the file will be in the same directory as the executable.
    lFilename = "D:\\Inception\\Content\\Models\\HornetAttackGL.FBX";

    // Initialize the exporter.
    bool lExportStatus = lExporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings());

    if (!lExportStatus) {
        printf("Call to FbxExporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
        return false;
    }

    lExporter->Export(lScene);

}*/
