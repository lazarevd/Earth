#include "EarthClass.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine.h"

#define PLANE_COUNT 6
#define M_PI 3.14159265358979323846  /* pi */

DEFINE_LOG_CATEGORY_STATIC(Earth, All, All)

// By Sebastian Lague tutorial
AEarthClass::AEarthClass()
{
    ThisScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = ThisScene;

    ThisMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GenerateMesh"));
    ThisMesh->SetupAttachment(RootComponent);

}

void AEarthClass::PostActorCreated()
{
    Super::PostActorCreated();
    GenerateMesh();
}

void AEarthClass::PostLoad()
{
    Super::PostLoad();
    GenerateMesh();
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Actors: %d"), AllActors.Num()));
}

void AEarthClass::GenerateMesh()
{
    Vertices.Reset();
    Triangles.Reset();
    Normals.Reset();
    Tangents.Reset();
    UVs.Reset();
    Colors.Reset();


    float scale = 100.0f;
    FProcMeshTangent TangentSetup = FProcMeshTangent(0.f, 1.f, 0.f);
    Vertices.SetNum(SphereResolution * SphereResolution * PLANE_COUNT);
    Triangles.SetNum((SphereResolution - 1) * (SphereResolution - 1) * 6 * PLANE_COUNT);
    Normals.SetNum((SphereResolution - 1) * (SphereResolution - 1) * 2 * PLANE_COUNT);
    Tangents.SetNum((SphereResolution - 1) * (SphereResolution - 1) * 2 * PLANE_COUNT);
    Colors.SetNum((SphereResolution - 1) * (SphereResolution - 1) * 2 * PLANE_COUNT);
    UVs.SetNum((SphereResolution - 1) * (SphereResolution - 1) * 6 * PLANE_COUNT);

    AddSquareMesh(FVector(0.0f, 1.0f, 0.0f), SphereResolution, 0, TangentSetup);
    AddSquareMesh(FVector(0.0f, -1.0f, 0.0f), SphereResolution, 1, TangentSetup);
    AddSquareMesh(FVector(1.0f, 0.0f, 0.0f), SphereResolution, 2, TangentSetup);
    AddSquareMesh(FVector(-1.0f, 0.0f, 0.0f), SphereResolution, 3, TangentSetup);
    AddSquareMesh(FVector(0.0f, 0.0f, 1.0f), SphereResolution, 4, TangentSetup);
    AddSquareMesh(FVector(0.0f, 0.0f, -1.0f), SphereResolution, 5, TangentSetup);

    for (int i = 0; i < Vertices.Num(); i++) {
        //Vertices[i] = Vertices[i].GetSafeNormal() * scale;
        Vertices[i] = PointOnCurveToPointOnSphere(Vertices[i]) * scale;
        UVs[i] = GenerateUV(Vertices[i]);
    }

    ThisMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, true);
}

void AEarthClass::AddSquareMesh(FVector normal, int32 resolution, int32 planeNumber, FProcMeshTangent Tangent)
{
    FVector axisA = FVector(normal.Y, normal.Z, normal.X);//WTF Sebastian Lague says to do so (Procedural planets (E01))
    FVector axisB = FVector::CrossProduct(normal, axisA).GetSafeNormal() * -1;


    int32 triIndex = 0;
    int32 triangleCount = 0;
    if (planeNumber != 0) {
        triIndex += (resolution - 1) * (resolution - 1) * 2 * 3 * planeNumber;// res * 2 triangles per poly * 3 vertice per triangle 
        triangleCount += (resolution - 1) * (resolution - 1) * 2 * planeNumber;
    }

    for (int x = 0; x < resolution; x++) {
        for (int y = 0; y < resolution; y++) {
            int32 vertexIndex = (x + y * resolution) + resolution * resolution * planeNumber;
            FVector2D gridPos = FVector2D(x, y) / (resolution - 1.0f);
            FVector point = normal + axisA * (2 * gridPos.X - 1) + axisB * (2 * gridPos.Y - 1);
            Vertices[vertexIndex] = point;

            if (x != resolution - 1 && y != resolution - 1) {
                Triangles[triIndex + 0] = vertexIndex;
                Triangles[triIndex + 1] = vertexIndex + resolution + 1;
                Triangles[triIndex + 2] = vertexIndex + resolution;
                triIndex += 3;
                Normals[triangleCount] = normal;
                Tangents[triangleCount] = Tangent;
                Colors[triangleCount] = FLinearColor::Green;
                triangleCount += 1;
                Triangles[triIndex + 0] = vertexIndex;
                Triangles[triIndex + 1] = vertexIndex + 1;
                Triangles[triIndex + 2] = vertexIndex + resolution + 1;
                triIndex += 3;
                Normals[triangleCount] = normal;
                Tangents[triangleCount] = Tangent;
                Colors[triangleCount] = FLinearColor::Green;
                triangleCount += 1;
            }
        }
    }
}

FVector AEarthClass::PointOnCurveToPointOnSphere(FVector p)
{
    float x2 = p.X * p.X;
    float y2 = p.Y * p.Y;
    float z2 = p.Z * p.Z;

    float x = p.X * FMath::Sqrt(1 - (y2 + z2) / 2 + (y2 * z2) / 3);
    float y = p.Y * FMath::Sqrt(1 - (x2 + z2) / 2 + (x2 * z2) / 3);
    float z = p.Z * FMath::Sqrt(1 - (x2 + y2) / 2 + (x2 * y2) / 3);
    return FVector(x, y, z);
}

FVector2D AEarthClass::GenerateUV(FVector p) {
    FVector centre = FVector(0.0, 0.0, 0.0);
    FVector d = (p - centre);
    d.Normalize();
    float u = 0.5f + UKismetMathLibrary::Atan2(d.Y, d.X) / (2.0f * M_PI);
    float v = 0.5f - UKismetMathLibrary::Asin(d.Z) / M_PI;
    UE_LOG(Earth, Warning, TEXT("u, v: %f, %f"), u, v);
    return FVector2D(u, v);
}

