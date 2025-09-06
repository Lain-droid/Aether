#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "PatternDetector.h"
#include "../security/XorStr.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>
#include <thread>

namespace AetherVisor {
    namespace ML {

        PatternDetector::PatternDetector() {
            // Initialize neural networks for different pattern types
            m_generalDetector = std::make_unique<NeuralNetwork>();
            m_anomalyDetector = std::make_unique<NeuralNetwork>();

            // Build general detector architecture
            m_generalDetector->addLayer(std::make_unique<Layer>(50, 128, &ActivationFunctions::relu));
            m_generalDetector->addLayer(std::make_unique<Layer>(128, 64, &ActivationFunctions::relu));
            m_generalDetector->addLayer(std::make_unique<Layer>(64, 32, &ActivationFunctions::relu));
            m_generalDetector->addLayer(std::make_unique<Layer>(32, 8, &ActivationFunctions::softmax));
            m_generalDetector->setOptimizer(std::make_unique<AdamOptimizer>(0.001));

            // Build anomaly detector
            m_anomalyDetector->addLayer(std::make_unique<Layer>(30, 64, &ActivationFunctions::relu));
            m_anomalyDetector->addLayer(std::make_unique<Layer>(64, 32, &ActivationFunctions::relu));
            m_anomalyDetector->addLayer(std::make_unique<Layer>(32, 16, &ActivationFunctions::relu));
            m_anomalyDetector->addLayer(std::make_unique<Layer>(16, 1, &ActivationFunctions::sigmoid));
            m_anomalyDetector->setOptimizer(std::make_unique<AdamOptimizer>(0.0005));

            // Initialize pattern-specific networks
            for (int i = 0; i < 8; ++i) {
                PatternType type = static_cast<PatternType>(i);
                auto network = std::make_unique<NeuralNetwork>();
                
                network->addLayer(std::make_unique<Layer>(40, 80, &ActivationFunctions::relu));
                network->addLayer(std::make_unique<Layer>(80, 40, &ActivationFunctions::relu));
                network->addLayer(std::make_unique<Layer>(40, 20, &ActivationFunctions::relu));
                network->addLayer(std::make_unique<Layer>(20, 1, &ActivationFunctions::sigmoid));
                network->setOptimizer(std::make_unique<AdamOptimizer>(0.001));
                
                m_patternNetworks[type] = std::move(network);
            }

            InitializePatternSignatures();
        }

        std::vector<PatternAnalysisResult> PatternDetector::AnalyzeEventStream(
            const std::vector<Backend::AIEventType>& events,
            const std::vector<std::chrono::time_point<std::chrono::steady_clock>>& timestamps) {
            
            std::vector<PatternAnalysisResult> results;
            
            if (events.size() != timestamps.size() || events.empty()) {
                return results;
            }

            // Extract comprehensive features
            std::vector<double> temporal_features = ExtractTemporalFeatures(events, timestamps);
            std::vector<double> sequential_features = ExtractSequentialFeatures(events);
            std::vector<double> statistical_features = ExtractStatisticalFeatures(events);

            // Combine all features
            std::vector<double> combined_features;
            combined_features.insert(combined_features.end(), temporal_features.begin(), temporal_features.end());
            combined_features.insert(combined_features.end(), sequential_features.begin(), sequential_features.end());
            combined_features.insert(combined_features.end(), statistical_features.begin(), statistical_features.end());

            // Ensure feature vector is the correct size
            combined_features.resize(50, 0.0);

            // Test against all known signatures
            for (const auto& [type, signature] : m_knownSignatures) {
                PatternAnalysisResult result = DetectPattern(signature, combined_features);
                if (result.confidence_score > signature.confidence_threshold) {
                    result.detected_pattern = type;
                    result.detection_time = std::chrono::steady_clock::now();
                    result.triggering_events = events;
                    result.is_critical = (result.confidence_score > 0.8);
                    
                    // Add pattern-specific description
                    switch (type) {
                        case PatternType::ANTI_CHEAT_SCAN:
                            result.description = XorS("Active anti-cheat scanning detected");
                            break;
                        case PatternType::MEMORY_PROBE:
                            result.description = XorS("Memory probing pattern identified");
                            break;
                        case PatternType::NETWORK_MONITORING:
                            result.description = XorS("Network monitoring behavior detected");
                            break;
                        case PatternType::BEHAVIORAL_ANALYSIS:
                            result.description = XorS("Behavioral analysis system active");
                            break;
                        default:
                            result.description = XorS("Unknown pattern detected");
                            break;
                    }
                    
                    results.push_back(result);
                }
            }

            // Use neural network for additional detection
            Matrix feature_matrix(1, combined_features.size());
            for (size_t i = 0; i < combined_features.size(); ++i) {
                feature_matrix.at(0, i) = combined_features[i];
            }

            Matrix prediction = m_generalDetector->predict(feature_matrix);
            
            // Find the most likely pattern type
            int max_index = 0;
            double max_confidence = prediction.at(0, 0);
            for (int i = 1; i < prediction.getCols(); ++i) {
                if (prediction.at(0, i) > max_confidence) {
                    max_confidence = prediction.at(0, i);
                    max_index = i;
                }
            }

            if (max_confidence > 0.7) {
                PatternAnalysisResult neural_result;
                neural_result.detected_pattern = static_cast<PatternType>(max_index);
                neural_result.confidence_score = max_confidence;
                neural_result.detection_time = std::chrono::steady_clock::now();
                neural_result.triggering_events = events;
                neural_result.is_critical = (max_confidence > 0.9);
                neural_result.description = XorS("Neural network pattern detection");
                
                results.push_back(neural_result);
            }

            return results;
        }

        PatternAnalysisResult PatternDetector::DetectPattern(const PatternSignature& signature,
                                                           const std::vector<double>& current_features) {
            PatternAnalysisResult result;
            result.confidence_score = 0.0;
            result.detected_pattern = signature.type;

            if (current_features.size() != signature.feature_vector.size()) {
                return result;
            }

            // Calculate similarity using multiple metrics
            double euclidean_similarity = 0.0;
            double cosine_similarity = 0.0;
            double correlation = 0.0;

            // Euclidean distance
            double sum_sq_diff = 0.0;
            for (size_t i = 0; i < current_features.size(); ++i) {
                double diff = current_features[i] - signature.feature_vector[i];
                sum_sq_diff += diff * diff;
            }
            euclidean_similarity = 1.0 / (1.0 + sqrt(sum_sq_diff));

            // Cosine similarity
            double dot_product = 0.0, norm1 = 0.0, norm2 = 0.0;
            for (size_t i = 0; i < current_features.size(); ++i) {
                dot_product += current_features[i] * signature.feature_vector[i];
                norm1 += current_features[i] * current_features[i];
                norm2 += signature.feature_vector[i] * signature.feature_vector[i];
            }
            if (norm1 > 0 && norm2 > 0) {
                cosine_similarity = dot_product / (sqrt(norm1) * sqrt(norm2));
            }

            // Weighted combination of similarities
            result.confidence_score = 0.4 * euclidean_similarity + 0.6 * cosine_similarity;
            
            // Apply pattern-specific neural network if available
            if (m_patternNetworks.count(signature.type)) {
                Matrix features(1, std::min(current_features.size(), size_t(40)));
                for (int i = 0; i < features.getCols(); ++i) {
                    features.at(0, i) = current_features[i];
                }
                
                Matrix neural_prediction = m_patternNetworks[signature.type]->predict(features);
                double neural_confidence = neural_prediction.at(0, 0);
                
                // Blend traditional and neural confidences
                result.confidence_score = 0.3 * result.confidence_score + 0.7 * neural_confidence;
            }

            return result;
        }

        TemporalBehaviorProfile PatternDetector::AnalyzeBehavioralPatterns(
            const std::vector<Backend::AIEventType>& event_history,
            const std::vector<std::chrono::time_point<std::chrono::steady_clock>>& timestamps) {
            
            TemporalBehaviorProfile profile;
            
            if (event_history.empty() || timestamps.empty()) {
                return profile;
            }

            // Calculate action frequencies
            std::map<Backend::AIEventType, int> event_counts;
            for (const auto& event : event_history) {
                event_counts[event]++;
            }

            profile.action_frequencies.reserve(event_counts.size());
            for (const auto& [event, count] : event_counts) {
                profile.action_frequencies.push_back(static_cast<double>(count) / event_history.size());
            }

            // Calculate timing intervals
            profile.timing_intervals.reserve(timestamps.size() - 1);
            for (size_t i = 1; i < timestamps.size(); ++i) {
                auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps[i] - timestamps[i-1]);
                profile.timing_intervals.push_back(static_cast<double>(interval.count()));
            }

            // Calculate periodicity score using autocorrelation
            if (profile.timing_intervals.size() > 10) {
                std::vector<double> autocorrelation(profile.timing_intervals.size() / 2);
                for (size_t lag = 1; lag < autocorrelation.size(); ++lag) {
                    double sum = 0.0;
                    for (size_t i = lag; i < profile.timing_intervals.size(); ++i) {
                        sum += profile.timing_intervals[i] * profile.timing_intervals[i - lag];
                    }
                    autocorrelation[lag] = sum / (profile.timing_intervals.size() - lag);
                }
                
                profile.periodicity_score = *std::max_element(autocorrelation.begin(), autocorrelation.end());
            }

            // Calculate randomness entropy
            if (!profile.timing_intervals.empty()) {
                std::map<int, int> interval_buckets;
                for (double interval : profile.timing_intervals) {
                    int bucket = static_cast<int>(interval / 100.0); // 100ms buckets
                    interval_buckets[bucket]++;
                }
                
                double entropy = 0.0;
                for (const auto& [bucket, count] : interval_buckets) {
                    double p = static_cast<double>(count) / profile.timing_intervals.size();
                    if (p > 0) {
                        entropy -= p * log2(p);
                    }
                }
                profile.randomness_entropy = entropy;
            }

            // Build event timing distributions
            for (const auto& [event, count] : event_counts) {
                std::vector<double> event_timings;
                for (size_t i = 0; i < event_history.size(); ++i) {
                    if (event_history[i] == event && i > 0) {
                        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
                            timestamps[i] - timestamps[i-1]);
                        event_timings.push_back(static_cast<double>(interval.count()));
                    }
                }
                profile.event_timing_distributions[event] = event_timings;
            }

            return profile;
        }

        double PatternDetector::CalculateAnomalyScore(const std::vector<double>& current_behavior,
                                                    const TemporalBehaviorProfile& baseline) {
            if (current_behavior.empty()) return 0.0;

            // Prepare features for anomaly detection
            std::vector<double> features;
            
            // Add current behavior features
            features.insert(features.end(), current_behavior.begin(), current_behavior.end());
            
            // Add baseline comparison features
            if (!baseline.action_frequencies.empty()) {
                for (size_t i = 0; i < std::min(current_behavior.size(), baseline.action_frequencies.size()); ++i) {
                    features.push_back(abs(current_behavior[i] - baseline.action_frequencies[i]));
                }
            }

            // Pad or truncate to expected size
            features.resize(30, 0.0);

            // Use anomaly detection neural network
            Matrix feature_matrix(1, features.size());
            for (size_t i = 0; i < features.size(); ++i) {
                feature_matrix.at(0, i) = features[i];
            }

            Matrix anomaly_score = m_anomalyDetector->predict(feature_matrix);
            return anomaly_score.at(0, 0);
        }

        std::vector<Backend::AIEventType> PatternDetector::GenerateEvasionSequence(PatternType detected_pattern) {
            std::vector<Backend::AIEventType> evasion_sequence;
            
            // Generate pattern-specific evasion strategies
            switch (detected_pattern) {
                case PatternType::ANTI_CHEAT_SCAN:
                    // Inject legitimate-looking events to mask suspicious activity
                    evasion_sequence = {
                        Backend::AIEventType::SCRIPT_EXECUTION,
                        Backend::AIEventType::MEMORY_READ,
                        Backend::AIEventType::NETWORK_PACKET_SENT,
                        Backend::AIEventType::PATTERN_LEARNING, // AI learning event
                        Backend::AIEventType::MEMORY_READ
                    };
                    break;
                    
                case PatternType::MEMORY_PROBE:
                    // Spread memory operations over time
                    evasion_sequence = {
                        Backend::AIEventType::SCRIPT_EXECUTION,
                        Backend::AIEventType::NEURAL_PREDICTION,
                        Backend::AIEventType::MEMORY_READ,
                        Backend::AIEventType::SCRIPT_EXECUTION,
                        Backend::AIEventType::MEMORY_READ
                    };
                    break;
                    
                case PatternType::BEHAVIORAL_ANALYSIS:
                    // Introduce more human-like variance
                    evasion_sequence = {
                        Backend::AIEventType::ADAPTIVE_BEHAVIOR_CHANGE,
                        Backend::AIEventType::SCRIPT_EXECUTION,
                        Backend::AIEventType::PATTERN_LEARNING,
                        Backend::AIEventType::NEURAL_PREDICTION
                    };
                    break;
                    
                default:
                    // General evasion strategy
                    evasion_sequence = GenerateDecoyEvents(5);
                    break;
            }

            return evasion_sequence;
        }

        void PatternDetector::StartRealTimeMonitoring() {
            m_realtimeMonitoring = true;
            m_realtimeEventBuffer.clear();
            m_realtimeTimestamps.clear();
            
            // Start monitoring thread (simplified)
            std::thread([this]() {
                while (m_realtimeMonitoring) {
                    if (m_realtimeEventBuffer.size() >= 10) {
                        // Analyze recent events
                        auto results = AnalyzeEventStream(m_realtimeEventBuffer, m_realtimeTimestamps);
                        
                        for (const auto& result : results) {
                            if (result.is_critical) {
                                // Trigger evasion measures
                                AdaptToDetectedPattern(result.detected_pattern);
                            }
                        }
                        
                        // Keep only recent events
                        if (m_realtimeEventBuffer.size() > 50) {
                            m_realtimeEventBuffer.erase(m_realtimeEventBuffer.begin(), 
                                                      m_realtimeEventBuffer.begin() + 25);
                            m_realtimeTimestamps.erase(m_realtimeTimestamps.begin(),
                                                     m_realtimeTimestamps.begin() + 25);
                        }
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }).detach();
        }

        void PatternDetector::ProcessRealtimeEvent(Backend::AIEventType event) {
            if (!m_realtimeMonitoring) return;
            
            m_realtimeEventBuffer.push_back(event);
            m_realtimeTimestamps.push_back(std::chrono::steady_clock::now());
        }

        std::vector<double> PatternDetector::ExtractTemporalFeatures(
            const std::vector<Backend::AIEventType>& events,
            const std::vector<std::chrono::time_point<std::chrono::steady_clock>>& timestamps) {
            
            std::vector<double> features;
            
            if (events.size() != timestamps.size() || events.empty()) {
                return features;
            }

            // Calculate timing intervals
            std::vector<double> intervals;
            for (size_t i = 1; i < timestamps.size(); ++i) {
                auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps[i] - timestamps[i-1]);
                intervals.push_back(static_cast<double>(interval.count()));
            }

            if (intervals.empty()) return features;

            // Basic statistical features
            double mean = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();
            features.push_back(mean);

            // Variance
            double variance = 0.0;
            for (double interval : intervals) {
                variance += (interval - mean) * (interval - mean);
            }
            variance /= intervals.size();
            features.push_back(variance);

            // Standard deviation
            features.push_back(sqrt(variance));

            // Min and max intervals
            auto minmax = std::minmax_element(intervals.begin(), intervals.end());
            features.push_back(*minmax.first);
            features.push_back(*minmax.second);

            // Periodicity analysis (simplified autocorrelation)
            if (intervals.size() > 5) {
                double autocorr = 0.0;
                for (size_t lag = 1; lag < std::min(intervals.size() / 2, size_t(10)); ++lag) {
                    double corr = 0.0;
                    for (size_t i = lag; i < intervals.size(); ++i) {
                        corr += intervals[i] * intervals[i - lag];
                    }
                    autocorr = std::max(autocorr, corr / (intervals.size() - lag));
                }
                features.push_back(autocorr);
            } else {
                features.push_back(0.0);
            }

            // Trend analysis (linear regression slope)
            double slope = 0.0;
            if (intervals.size() > 2) {
                double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
                for (size_t i = 0; i < intervals.size(); ++i) {
                    sum_x += i;
                    sum_y += intervals[i];
                    sum_xy += i * intervals[i];
                    sum_xx += i * i;
                }
                slope = (intervals.size() * sum_xy - sum_x * sum_y) / 
                       (intervals.size() * sum_xx - sum_x * sum_x);
            }
            features.push_back(slope);

            return features;
        }

        std::vector<double> PatternDetector::ExtractSequentialFeatures(const std::vector<Backend::AIEventType>& events) {
            std::vector<double> features;
            
            if (events.empty()) return features;

            // Event type frequencies
            std::map<Backend::AIEventType, int> event_counts;
            for (const auto& event : events) {
                event_counts[event]++;
            }

            // Convert to frequency features (normalized)
            for (int i = 0; i < 21; ++i) { // Assuming 21 different event types
                Backend::AIEventType event_type = static_cast<Backend::AIEventType>(i);
                double frequency = static_cast<double>(event_counts[event_type]) / events.size();
                features.push_back(frequency);
            }

            // Sequence pattern features
            if (events.size() > 1) {
                // Transition probabilities (simplified)
                std::map<std::pair<Backend::AIEventType, Backend::AIEventType>, int> transitions;
                for (size_t i = 1; i < events.size(); ++i) {
                    transitions[{events[i-1], events[i]}]++;
                }
                
                // Calculate entropy of transitions
                double transition_entropy = 0.0;
                for (const auto& [transition, count] : transitions) {
                    double p = static_cast<double>(count) / (events.size() - 1);
                    if (p > 0) {
                        transition_entropy -= p * log2(p);
                    }
                }
                features.push_back(transition_entropy);
            } else {
                features.push_back(0.0);
            }

            return features;
        }

        std::vector<double> PatternDetector::ExtractStatisticalFeatures(const std::vector<Backend::AIEventType>& events) {
            std::vector<double> features;
            
            if (events.empty()) return features;

            // Convert events to numeric sequence for analysis
            std::vector<double> numeric_sequence;
            for (const auto& event : events) {
                numeric_sequence.push_back(static_cast<double>(event));
            }

            // Calculate statistical moments
            double mean = std::accumulate(numeric_sequence.begin(), numeric_sequence.end(), 0.0) / numeric_sequence.size();
            features.push_back(mean);

            // Variance and standard deviation
            double variance = 0.0;
            for (double val : numeric_sequence) {
                variance += (val - mean) * (val - mean);
            }
            variance /= numeric_sequence.size();
            features.push_back(variance);
            features.push_back(sqrt(variance));

            // Skewness (third moment)
            double skewness = 0.0;
            double std_dev = sqrt(variance);
            if (std_dev > 0) {
                for (double val : numeric_sequence) {
                    skewness += pow((val - mean) / std_dev, 3);
                }
                skewness /= numeric_sequence.size();
            }
            features.push_back(skewness);

            // Kurtosis (fourth moment)
            double kurtosis = 0.0;
            if (std_dev > 0) {
                for (double val : numeric_sequence) {
                    kurtosis += pow((val - mean) / std_dev, 4);
                }
                kurtosis = kurtosis / numeric_sequence.size() - 3.0; // Excess kurtosis
            }
            features.push_back(kurtosis);

            return features;
        }

        void PatternDetector::InitializePatternSignatures() {
            // Initialize known anti-cheat patterns
            PatternSignature anti_cheat_sig;
            anti_cheat_sig.type = PatternType::ANTI_CHEAT_SCAN;
            anti_cheat_sig.confidence_threshold = 0.7;
            anti_cheat_sig.feature_vector = {
                0.8, 0.1, 0.05, 0.02, 0.01, 0.02, 0.0, // Event frequencies
                2.5, 150.0, 50.0, 10.0, 500.0, 0.3, 0.1, // Temporal features
                5.2, 1.2, 2.1, 0.8, -0.3 // Statistical features
            };
            // Pad to consistent size
            anti_cheat_sig.feature_vector.resize(50, 0.0);
            m_knownSignatures[PatternType::ANTI_CHEAT_SCAN] = anti_cheat_sig;

            // Memory probe pattern
            PatternSignature memory_sig;
            memory_sig.type = PatternType::MEMORY_PROBE;
            memory_sig.confidence_threshold = 0.65;
            memory_sig.feature_vector = {
                0.1, 0.6, 0.2, 0.05, 0.02, 0.01, 0.02, // Event frequencies
                1.8, 80.0, 30.0, 5.0, 200.0, 0.2, 0.05, // Temporal features
                8.5, 2.1, 1.8, 0.6, 0.2 // Statistical features
            };
            memory_sig.feature_vector.resize(50, 0.0);
            m_knownSignatures[PatternType::MEMORY_PROBE] = memory_sig;

            // Network monitoring pattern
            PatternSignature network_sig;
            network_sig.type = PatternType::NETWORK_MONITORING;
            network_sig.confidence_threshold = 0.6;
            network_sig.feature_vector = {
                0.05, 0.1, 0.05, 0.02, 0.01, 0.7, 0.07, // Event frequencies
                3.2, 200.0, 80.0, 20.0, 1000.0, 0.4, 0.15, // Temporal features
                3.8, 0.9, 1.2, 0.3, -0.1 // Statistical features
            };
            network_sig.feature_vector.resize(50, 0.0);
            m_knownSignatures[PatternType::NETWORK_MONITORING] = network_sig;

            // Behavioral analysis pattern
            PatternSignature behavioral_sig;
            behavioral_sig.type = PatternType::BEHAVIORAL_ANALYSIS;
            behavioral_sig.confidence_threshold = 0.75;
            behavioral_sig.feature_vector = {
                0.2, 0.15, 0.1, 0.3, 0.1, 0.1, 0.05, // Event frequencies
                1.5, 120.0, 40.0, 8.0, 300.0, 0.25, 0.08, // Temporal features
                6.2, 1.8, 2.5, 0.9, 0.1 // Statistical features
            };
            behavioral_sig.feature_vector.resize(50, 0.0);
            m_knownSignatures[PatternType::BEHAVIORAL_ANALYSIS] = behavioral_sig;
        }

        void PatternDetector::AdaptToDetectedPattern(PatternType pattern) {
            // Update detection counts
            m_detectionCounts[pattern]++;
            
            // Generate and execute evasion sequence
            auto evasion_sequence = GenerateEvasionSequence(pattern);
            
            // Inject steganographic noise
            auto noisy_sequence = InjectStealthyNoise(evasion_sequence);
            
            // Update baseline behavior to include evasion patterns
            UpdateMarkovChain(noisy_sequence);
            
            // Adjust sensitivity thresholds
            if (m_knownSignatures.count(pattern)) {
                m_knownSignatures[pattern].confidence_threshold += 0.05; // Become less sensitive
                m_knownSignatures[pattern].confidence_threshold = std::min(0.95, m_knownSignatures[pattern].confidence_threshold);
            }
        }

        std::vector<Backend::AIEventType> PatternDetector::InjectStealthyNoise(
            const std::vector<Backend::AIEventType>& base_sequence) {
            
            std::vector<Backend::AIEventType> noisy_sequence = base_sequence;
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> event_dis(0, 20); // Random event types
            std::uniform_real_distribution<> prob_dis(0.0, 1.0);
            
            // Inject random legitimate events with low probability
            for (size_t i = 0; i < noisy_sequence.size(); ++i) {
                if (prob_dis(gen) < 0.3) { // 30% chance to inject noise
                    Backend::AIEventType noise_event = static_cast<Backend::AIEventType>(event_dis(gen));
                    noisy_sequence.insert(noisy_sequence.begin() + i, noise_event);
                    ++i; // Skip the inserted element
                }
            }
            
            return noisy_sequence;
        }

        std::vector<Backend::AIEventType> PatternDetector::GenerateDecoyEvents(int count) {
            std::vector<Backend::AIEventType> decoys;
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 20);
            
            // Generate benign-looking events
            std::vector<Backend::AIEventType> benign_events = {
                Backend::AIEventType::SCRIPT_EXECUTION,
                Backend::AIEventType::PAYLOAD_EXECUTED,
                Backend::AIEventType::NETWORK_PACKET_SENT,
                Backend::AIEventType::NETWORK_PACKET_RECEIVED,
                Backend::AIEventType::NEURAL_PREDICTION,
                Backend::AIEventType::PATTERN_LEARNING
            };
            
            for (int i = 0; i < count; ++i) {
                int index = dis(gen) % benign_events.size();
                decoys.push_back(benign_events[index]);
            }
            
            return decoys;
        }

        void PatternDetector::UpdateMarkovChain(const std::vector<Backend::AIEventType>& events) {
            if (events.size() < 2) return;
            
            // Update transition probabilities
            for (size_t i = 1; i < events.size(); ++i) {
                std::vector<Backend::AIEventType> context;
                if (i >= 2) {
                    context = {events[i-2], events[i-1]};
                } else {
                    context = {events[i-1]};
                }
                
                m_markovChains[context][events[i]]++;
            }
            
            // Normalize probabilities
            for (auto& [context, transitions] : m_markovChains) {
                int total = 0;
                for (const auto& [event, count] : transitions) {
                    total += count;
                }
                
                if (total > 0) {
                    for (auto& [event, count] : transitions) {
                        // Convert counts to probabilities, but keep as int for simplicity
                        // In a real implementation, you'd use double here
                    }
                }
            }
        }

        double PatternDetector::PredictNextEventProbability(const std::vector<Backend::AIEventType>& context,
                                                          Backend::AIEventType next_event) {
            if (m_markovChains.count(context) == 0) {
                return 1.0 / 21.0; // Uniform probability for unknown context
            }
            
            const auto& transitions = m_markovChains[context];
            int total = 0;
            for (const auto& [event, count] : transitions) {
                total += count;
            }
            
            if (total == 0 || transitions.count(next_event) == 0) {
                return 1.0 / 21.0; // Smoothing
            }
            
            return static_cast<double>(transitions.at(next_event)) / total;
        }

    } // namespace ML
} // namespace AetherVisor