// Test file to verify merging behavior
#include "CoreMinimal.h"

class TestClass
{
public:
    //GEN-BEGIN: TestRegion
    int oldGenerated = 1;
    // This should be replaced
    //GEN-END: TestRegion
    
    int customVariable = 2; // This should be preserved
    
    //GEN-BEGIN: AnotherRegion
    float anotherGenerated = 3.0f;
    //GEN-END: AnotherRegion
};