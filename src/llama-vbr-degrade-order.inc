// GENERATED 2026-07-01 from weekly-runs/vbr_master_frontier_20260627/perside_order_*.json
// (Qwen3.6-27B Q6_K, measured tap-OFF — re-sweep tap-on later, see linked-soaring-fox.md).
// Global decode-time degrade order: 5 bands x 32 (layer,side) units, each band = REVERSE of
// order_protect_first (least-protected degrades first). F16->t8 band reuses the t8->t4 order
// (no measured json for that transition). Runtime override: VBR_DEGRADE_ORDER=<file> with
// whitespace-separated tokens "<il><k|v>:<t8|t4|t3|t2|t1>".
static const vbr_degrade_step vbr_degrade_order_baked[] = {
    // band -> t8 (from perside_order_t8t4.json, transition t8->t4)
    {55, 1, VBR_TIER_T8}, {47, 1, VBR_TIER_T8}, {51, 1, VBR_TIER_T8}, {43, 0, VBR_TIER_T8},
    {59, 1, VBR_TIER_T8}, {39, 0, VBR_TIER_T8}, {43, 1, VBR_TIER_T8}, {59, 0, VBR_TIER_T8},
    {55, 0, VBR_TIER_T8}, {51, 0, VBR_TIER_T8}, {39, 1, VBR_TIER_T8}, {63, 0, VBR_TIER_T8},
    {47, 0, VBR_TIER_T8}, {23, 1, VBR_TIER_T8}, {35, 1, VBR_TIER_T8}, {35, 0, VBR_TIER_T8},
    {27, 1, VBR_TIER_T8}, {63, 1, VBR_TIER_T8}, { 3, 0, VBR_TIER_T8}, { 7, 1, VBR_TIER_T8},
    {23, 0, VBR_TIER_T8}, {31, 1, VBR_TIER_T8}, {15, 1, VBR_TIER_T8}, {27, 0, VBR_TIER_T8},
    {19, 1, VBR_TIER_T8}, {31, 0, VBR_TIER_T8}, { 3, 1, VBR_TIER_T8}, {11, 1, VBR_TIER_T8},
    {19, 0, VBR_TIER_T8}, { 7, 0, VBR_TIER_T8}, {15, 0, VBR_TIER_T8}, {11, 0, VBR_TIER_T8},
    // band -> t4 (from perside_order_t8t4.json, transition t8->t4)
    {55, 1, VBR_TIER_T4}, {47, 1, VBR_TIER_T4}, {51, 1, VBR_TIER_T4}, {43, 0, VBR_TIER_T4},
    {59, 1, VBR_TIER_T4}, {39, 0, VBR_TIER_T4}, {43, 1, VBR_TIER_T4}, {59, 0, VBR_TIER_T4},
    {55, 0, VBR_TIER_T4}, {51, 0, VBR_TIER_T4}, {39, 1, VBR_TIER_T4}, {63, 0, VBR_TIER_T4},
    {47, 0, VBR_TIER_T4}, {23, 1, VBR_TIER_T4}, {35, 1, VBR_TIER_T4}, {35, 0, VBR_TIER_T4},
    {27, 1, VBR_TIER_T4}, {63, 1, VBR_TIER_T4}, { 3, 0, VBR_TIER_T4}, { 7, 1, VBR_TIER_T4},
    {23, 0, VBR_TIER_T4}, {31, 1, VBR_TIER_T4}, {15, 1, VBR_TIER_T4}, {27, 0, VBR_TIER_T4},
    {19, 1, VBR_TIER_T4}, {31, 0, VBR_TIER_T4}, { 3, 1, VBR_TIER_T4}, {11, 1, VBR_TIER_T4},
    {19, 0, VBR_TIER_T4}, { 7, 0, VBR_TIER_T4}, {15, 0, VBR_TIER_T4}, {11, 0, VBR_TIER_T4},
    // band -> t3 (from perside_order_t4t3.json, transition t4->t3)
    {35, 1, VBR_TIER_T3_TCQ}, {15, 0, VBR_TIER_T3_TCQ}, {47, 1, VBR_TIER_T3_TCQ}, {51, 1, VBR_TIER_T3_TCQ},
    {39, 0, VBR_TIER_T3_TCQ}, {55, 1, VBR_TIER_T3_TCQ}, {51, 0, VBR_TIER_T3_TCQ}, {59, 1, VBR_TIER_T3_TCQ},
    {59, 0, VBR_TIER_T3_TCQ}, {43, 1, VBR_TIER_T3_TCQ}, {63, 0, VBR_TIER_T3_TCQ}, {39, 1, VBR_TIER_T3_TCQ},
    {55, 0, VBR_TIER_T3_TCQ}, {47, 0, VBR_TIER_T3_TCQ}, {43, 0, VBR_TIER_T3_TCQ}, {31, 1, VBR_TIER_T3_TCQ},
    {35, 0, VBR_TIER_T3_TCQ}, {23, 1, VBR_TIER_T3_TCQ}, {63, 1, VBR_TIER_T3_TCQ}, {15, 1, VBR_TIER_T3_TCQ},
    {23, 0, VBR_TIER_T3_TCQ}, { 7, 1, VBR_TIER_T3_TCQ}, {27, 1, VBR_TIER_T3_TCQ}, {31, 0, VBR_TIER_T3_TCQ},
    {19, 0, VBR_TIER_T3_TCQ}, { 3, 1, VBR_TIER_T3_TCQ}, {11, 0, VBR_TIER_T3_TCQ}, {27, 0, VBR_TIER_T3_TCQ},
    {11, 1, VBR_TIER_T3_TCQ}, {19, 1, VBR_TIER_T3_TCQ}, { 7, 0, VBR_TIER_T3_TCQ}, { 3, 0, VBR_TIER_T3_TCQ},
    // band -> t2 (from perside_order_t3t2_q6k.json, transition t3->t2)
    {43, 1, VBR_TIER_T2_TCQ}, {55, 1, VBR_TIER_T2_TCQ}, {51, 1, VBR_TIER_T2_TCQ}, {39, 1, VBR_TIER_T2_TCQ},
    {59, 1, VBR_TIER_T2_TCQ}, {55, 0, VBR_TIER_T2_TCQ}, {35, 1, VBR_TIER_T2_TCQ}, {47, 1, VBR_TIER_T2_TCQ},
    {59, 0, VBR_TIER_T2_TCQ}, {63, 0, VBR_TIER_T2_TCQ}, {47, 0, VBR_TIER_T2_TCQ}, {51, 0, VBR_TIER_T2_TCQ},
    {39, 0, VBR_TIER_T2_TCQ}, { 3, 1, VBR_TIER_T2_TCQ}, {19, 1, VBR_TIER_T2_TCQ}, {43, 0, VBR_TIER_T2_TCQ},
    {15, 1, VBR_TIER_T2_TCQ}, {23, 1, VBR_TIER_T2_TCQ}, {35, 0, VBR_TIER_T2_TCQ}, {27, 1, VBR_TIER_T2_TCQ},
    {19, 0, VBR_TIER_T2_TCQ}, {31, 1, VBR_TIER_T2_TCQ}, { 3, 0, VBR_TIER_T2_TCQ}, {31, 0, VBR_TIER_T2_TCQ},
    {23, 0, VBR_TIER_T2_TCQ}, {11, 1, VBR_TIER_T2_TCQ}, {63, 1, VBR_TIER_T2_TCQ}, {27, 0, VBR_TIER_T2_TCQ},
    { 7, 1, VBR_TIER_T2_TCQ}, { 7, 0, VBR_TIER_T2_TCQ}, {15, 0, VBR_TIER_T2_TCQ}, {11, 0, VBR_TIER_T2_TCQ},
    // band -> t1 (from perside_order_t2t1.json, transition t2->t1)
    {23, 1, VBR_TIER_T1_TCQ}, {35, 1, VBR_TIER_T1_TCQ}, {23, 0, VBR_TIER_T1_TCQ}, {31, 1, VBR_TIER_T1_TCQ},
    { 7, 1, VBR_TIER_T1_TCQ}, {27, 1, VBR_TIER_T1_TCQ}, {39, 1, VBR_TIER_T1_TCQ}, {15, 1, VBR_TIER_T1_TCQ},
    {59, 1, VBR_TIER_T1_TCQ}, {51, 1, VBR_TIER_T1_TCQ}, {47, 1, VBR_TIER_T1_TCQ}, {19, 1, VBR_TIER_T1_TCQ},
    {55, 1, VBR_TIER_T1_TCQ}, {39, 0, VBR_TIER_T1_TCQ}, {15, 0, VBR_TIER_T1_TCQ}, {43, 1, VBR_TIER_T1_TCQ},
    {47, 0, VBR_TIER_T1_TCQ}, {55, 0, VBR_TIER_T1_TCQ}, {11, 1, VBR_TIER_T1_TCQ}, { 3, 1, VBR_TIER_T1_TCQ},
    {43, 0, VBR_TIER_T1_TCQ}, {59, 0, VBR_TIER_T1_TCQ}, {19, 0, VBR_TIER_T1_TCQ}, {63, 0, VBR_TIER_T1_TCQ},
    { 3, 0, VBR_TIER_T1_TCQ}, {51, 0, VBR_TIER_T1_TCQ}, {11, 0, VBR_TIER_T1_TCQ}, { 7, 0, VBR_TIER_T1_TCQ},
    {35, 0, VBR_TIER_T1_TCQ}, {31, 0, VBR_TIER_T1_TCQ}, {27, 0, VBR_TIER_T1_TCQ}, {63, 1, VBR_TIER_T1_TCQ},
};
