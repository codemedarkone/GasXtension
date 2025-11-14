// Test file for guarded region preservation
#pragma once

//GEN-BEGIN:Includes
// This should be replaced during generation
#include "OldInclude.h"
//GEN-END:Includes

// This custom code should survive generation
#include "MyCustomInclude.h"

//GEN-BEGIN:AttributeSet
// Old generated AttributeSet that should be replaced
UCLASS()
class UOldTestAttributes : public UAttributeSet
{
    GENERATED_BODY()
    
public:
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_OldHealth)
    FGameplayAttributeData OldHealth;
    ATTRIBUTE_ACCESSORS(UOldTestAttributes, OldHealth);
    
private:
    UFUNCTION()
    void OnRep_OldHealth(const FGameplayAttributeData& OldValue);
};
//GEN-END:AttributeSet

// Custom function that should survive generation
void CustomFunctionThatShouldSurvive()
{
    // This code should not be touched by the generator
    UE_LOG(LogTemp, Log, TEXT("Custom function preserved!"));
}

//GEN-BEGIN:Functions
// Old generated functions that should be replaced
void OldGeneratedFunction()
{
    // This should be completely replaced
}
//GEN-END:Functions

// Another custom section that should survive
namespace CustomNamespace
{
    // This should also be preserved
    constexpr float kCustomConstant = 100.0f;
}