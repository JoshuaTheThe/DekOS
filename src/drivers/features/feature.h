#ifndef FEATURE_H
#define FEATURE_H

#include<utils.h>

void FeatureCR0Enable(uint32_t FeatureIdx);
void FeatureCR4Enable(uint32_t FeatureIdx);
void SetCR3(uint32_t Value);
uint32_t GetCR3();

bool FeatureIsPresentEDX(uint32_t FeatureIdx);
bool FeatureIsPresentECX(uint32_t FeatureIdx);
bool *FindFeatures(size_t *Count);
char *FeatureName(size_t Index);
void MSRGet(uint32_t msr, uint32_t *lo, uint32_t *hi);
void MSRSet(uint32_t msr, uint32_t lo, uint32_t hi);
void FeaturesInit(void);
bool IsFeaturePresent(size_t Idx);

#endif
