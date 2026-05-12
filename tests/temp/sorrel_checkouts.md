# Sorrel Checkout: Empirical Audit Results
STATUS [FOUND] CloudSystem::authenticateVault -> src/cloud/cloud.cpp:44
STATUS [FOUND] CloudSystem::fetchRemoteConfig -> src/cloud/cloud.cpp:24
STATUS [FOUND] CloudSystem::publishToQueue -> src/cloud/cloud.cpp:37
STATUS [FOUND] CloudSystem::streamToKinesis -> src/cloud/cloud.cpp:30
STATUS [FOUND] CloudSystem::syncToS3 -> src/cloud/cloud.cpp:10
STATUS [FOUND] CloudSystem::triggerLambda -> src/cloud/cloud.cpp:17
STATUS [FOUND] DBConnector::queryHistory -> src/cloud/cloud.cpp:54
STATUS [FOUND] DBConnector::saveFrame -> src/cloud/cloud.cpp:48
STATUS [FOUND] GRPCInterface::deserialize -> src/cloud/cloud.cpp:79
STATUS [FOUND] GRPCInterface::serialize -> src/cloud/cloud.cpp:67
STATUS [FOUND] JsonValue::parse -> src/core/pathway_logic.cpp:132
STATUS [FOUND] MenuManager::run -> src/ui/ui_controller.cpp:123
STATUS [FOUND] P2PSystem::broadcast -> src/cloud/cloud.cpp:93
STATUS [FOUND] RegionAtlas::builtin -> src/core/atlas_core.cpp:111
STATUS [FOUND] applyASCIIShader -> src/visualization/video_logic.cpp:170
STATUS [FOUND] applyActivityDecayModel -> src/core/modeling_engine.cpp:34
STATUS [FOUND] applyClustering -> src/analytics/analytics.cpp:136
STATUS [FOUND] applyCustomMathematicalFunctions -> src/core/modeling_engine.cpp:81
STATUS [FOUND] applyDynamicPanning -> src/ai/ai.h:18
STATUS [FOUND] applyFrameInterpolationNN -> src/ai/ai.h:12
STATUS [FOUND] applyLongTermPotentiation -> src/core/modeling_engine.cpp:107
STATUS [FOUND] applyNeuralCA -> src/ai/ai.h:7
STATUS [FOUND] applyNeurotransmitterSimulation -> src/core/modeling_engine.cpp:92
STATUS [FOUND] applyPredictiveModeling -> src/ai/ai.h:13
STATUS [FOUND] applyRefractoryPeriodLogic -> src/core/modeling_engine.cpp:55
STATUS [FOUND] applyStochasticModeling -> src/core/modeling_engine.cpp:70
STATUS [FOUND] applyStyleGAN -> src/ai/ai.h:19
STATUS [FOUND] applyStyleTransfer -> src/ai/ai.h:8
STATUS [FOUND] applySynapticDelaySimulation -> src/core/modeling_engine.cpp:43
STATUS [FOUND] applyTemporalSmoothing -> src/core/modeling_engine.cpp:16
STATUS [FOUND] applyVideoDiffusion -> src/ai/ai.h:20
STATUS [MISSING] audit_memory_usage
STATUS [FOUND] calculateConnectivityMatrix -> src/analytics/analytics.cpp:130
STATUS [FOUND] calculateCrossCorrelation -> src/analytics/analytics.cpp:117
STATUS [FOUND] calculateEntropy -> src/analytics/analytics.cpp:86
STATUS [FOUND] calculateFFT -> src/analytics/analytics.cpp:111
STATUS [FOUND] calculateGrangerCausality -> src/analytics/analytics.cpp:97
STATUS [FOUND] calculateMutualInformation -> src/analytics/analytics.cpp:22
STATUS [FOUND] calculatePCA -> src/analytics/analytics.cpp:105
STATUS [MISSING] check_system_integrity
STATUS [MISSING] compressData
STATUS [FOUND] detectAnomalies -> src/ai/ai.h:17
STATUS [FOUND] detectEvents -> src/analytics/analytics.cpp:153
STATUS [FOUND] displayProgressBar -> src/ui/ui_controller.cpp:142
STATUS [MISSING] encryptData
STATUS [FOUND] findCrossCorrelationLag -> src/analytics/analytics.cpp:27
STATUS [FOUND] generateFrames -> src/visualization/video_logic.cpp:154
STATUS [FOUND] generateInputFromLLM -> src/ai/ai.h:15
STATUS [FOUND] generatePoeticDescription -> src/ai/ai.h:11
STATUS [FOUND] generateProceduralPattern -> src/ai/ai.h:9
STATUS [FOUND] generateStatisticsSummary -> src/analytics/analytics.cpp:140
STATUS [FOUND] getBrainStateTemplate -> src/core/modeling_engine.cpp:121
STATUS [FOUND] handleMouseClick -> src/ui/ui_controller.cpp:146
STATUS [FOUND] identifyPatterns -> src/ai/ai.h:10
STATUS [FOUND] intensityToChar -> src/visualization/video_logic.cpp:48
STATUS [FOUND] intensityToColor -> src/visualization/video_logic.cpp:145
STATUS [FOUND] intensityToSymbol -> src/visualization/video_logic.cpp:146
STATUS [FOUND] loadConfigJSON -> src/main.cpp:49
STATUS [MISSING] loadSimulationState
STATUS [FOUND] load_simulation_state -> src/ui/ui_controller.cpp:40
STATUS [MISSING] parseBrainActivityCSV
STATUS [MISSING] parseBrainActivityJSON
STATUS [MISSING] parseBrainActivityXML
STATUS [MISSING] parseBrainActivityYAML
STATUS [FOUND] parseConfigFromNaturalLanguage -> src/ai/ai.h:16
STATUS [FOUND] parse_frames_by_format -> src/main.cpp:96
STATUS [FOUND] parse_frames_csv -> src/core/data_parsing_hub.cpp:36
STATUS [FOUND] parse_frames_json -> src/core/data_parsing_hub.cpp:33
STATUS [FOUND] parse_frames_xml -> src/core/data_parsing_hub.cpp:35
STATUS [FOUND] parse_frames_yaml -> src/core/data_parsing_hub.cpp:34
STATUS [FOUND] performClustering -> src/analytics/analytics.cpp:18
STATUS [FOUND] performFFT -> src/analytics/analytics.cpp:114
STATUS [FOUND] performPCA -> src/analytics/analytics.cpp:108
STATUS [FOUND] render3D -> src/visualization/video_logic.cpp:162
STATUS [FOUND] render3DCortexNeRF -> src/ai/ai.h:14
STATUS [FOUND] renderCorrelationMatrix -> src/analytics/analytics.cpp:14
STATUS [FOUND] renderGrid -> src/visualization/video_logic.cpp:161
STATUS [FOUND] renderLargeText -> src/visualization/video_logic.cpp:182
STATUS [FOUND] renderParticles -> src/visualization/video_logic.cpp:163
STATUS [FOUND] renderRegion -> src/visualization/video_logic.cpp:164
STATUS [MISSING] saveSimulationState
STATUS [FOUND] save_simulation_state -> src/core/data_parsing_hub.cpp:47
STATUS [FOUND] setTheme -> src/ui/ui_controller.cpp:145
STATUS [FOUND] set_current_atlas -> src/main.cpp:78
STATUS [MISSING] streamRealtimeData
STATUS [FOUND] synthesizeRealTimeSound -> src/visualization/video_logic.cpp:178
STATUS [FOUND] toggleQuietMode -> src/ui/ui_controller.cpp:141
STATUS [FOUND] toggleVerboseMode -> src/ui/ui_controller.cpp:140
STATUS [MISSING] validateBrainActivityJSON
STATUS [MISSING] validate_atlas_schema
STATUS [MISSING] verify_resource_bounds

SUMMARY:
found_count = 77
missing_count = 14
