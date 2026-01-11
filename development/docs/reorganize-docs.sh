#!/bin/bash
# Documentation Reorganization Script
# Run from development/docs directory

set -e

echo "Creating directory structure..."
mkdir -p getting-started building workflow architecture scene-migration history testing planning archive

echo "Moving files to getting-started/..."
git mv QUICKSTART.md getting-started/
git mv GETTING_STARTED.md getting-started/
git mv DEVELOPMENT_SETUP.md getting-started/

echo "Moving files to building/..."
git mv BUILDING_BEES_GUIDE.md building/
git mv COLIMA_BUILD_FIX.md building/
git mv BEEKEEP_M1_BUILD_SUMMARY.md building/

echo "Moving files to workflow/..."
git mv DEVELOPMENT_CHECKLIST.md workflow/
git mv GIT_STRATEGY.md workflow/
git mv DEVELOPMENT_STABILITY_STRATEGY.md workflow/

echo "Moving files to architecture/..."
git mv memory-management-architecture-v1.0.md architecture/
git mv memory-pool-system-analysis.md architecture/
git mv graphics-memory-management-analysis.md architecture/
git mv fixed-point-math-performance-audit.md architecture/
git mv real-time-performance-baseline.md architecture/
git mv critical-fixme-comments-catalog.md architecture/

echo "Moving files to scene-migration/..."
git mv SCENE_MIGRATION_STRATEGY.md scene-migration/
git mv OPERATOR_ID_MAPPING_ANALYSIS.md scene-migration/
git mv OPERATOR_OUTPUT_ANALYSIS_SUMMARY.md scene-migration/
git mv OPERATOR_OUTPUT_ANALYSIS_RESULTS.md scene-migration/
git mv OPERATOR_OUTPUT_CHANGES_TODO.md scene-migration/
git mv SCENE_MIGRATION_PHASE1_COMPLETE.md scene-migration/

echo "Moving files to history/..."
git mv CDC_MERGE_COMPLETE_2026-01-11.md history/
git mv GIT_MIGRATION_COMPLETE_2026-01-11.md history/
git mv STABILITY_RECOVERY_2026-01-11.md history/

echo "Moving files to testing/..."
git mv TEST_RESULTS.md testing/
git mv BLACKFIN_TEST_RESULTS.md testing/
git mv CDC_DEBUG_GUIDE.md testing/

echo "Moving files to planning/..."
git mv aleph-bees-1.0-development-pathway.md planning/
git mv beekeep-development-plan.md planning/
git mv PHASE_0_EXECUTION_PLAN.md planning/

echo "Moving files to archive/..."
git mv ALEPH_DEVELOPER_GUIDE.md archive/
git mv ALEPH_DEVELOPMENT_ENVIRONMENT.md archive/
git mv README_DEV.md archive/
git mv quick-start-guide.md archive/
git mv bees-0.8-analysis.md archive/
git mv development-pathway-update-summary.md archive/

echo "Done! Review changes with: git status"
echo "Commit with: git commit -m 'Docs: Reorganize documentation into logical categories'"
