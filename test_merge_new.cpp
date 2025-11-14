// Test file to verify merging behavior
#include "CoreMinimal.h"

class TestClass
{
public:
    //GEN-BEGIN: TestRegion
    int newGenerated = 99;
    float newFloat = 42.0f;
    // This is new generated content
    //GEN-END: TestRegion
    
    //GEN-BEGIN: AnotherRegion
    double newDouble = 7.0;
    //GEN-END: AnotherRegion
};