// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "EarthClass.generated.h"


UCLASS()
class EARTH_API AEarthClass : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEarthClass();

protected:
	UPROPERTY(VisibleAnywhere)
		USceneComponent* ThisScene;
	UPROPERTY(VisibleAnywhere)
		UProceduralMeshComponent* ThisMesh;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "3", ClampMax = "10", UIMin = "3", UIMax = "10"))
		int32 SphereResolution = 3;

	virtual void PostActorCreated() override;
	virtual void PostLoad() override;
	void GenerateMesh();

private:	
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent>Tangents;
	TArray<FVector2D>UVs;
	TArray<FLinearColor>Colors;


	void AddSquareMesh(FVector normal, int32 resolution, int32 planeNumber, FProcMeshTangent Tangent);

	FVector PointOnCurveToPointOnSphere(FVector p);

	FVector2D GenerateUV(FVector p);

};
