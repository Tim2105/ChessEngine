#ifndef REN_INSTANCE_H
#define REN_INSTANCE_H

#include "core/utils/ren/Accumulator.h"
#include "core/utils/ren/RENNetwork.h"

#include <vector>

namespace REN {
    class Instance {
        private:
            struct alignas(REQUIRED_ALIGNMENT) AccumulatorData {
                int16_t accumulator[Network::REN_SIZE];

                constexpr AccumulatorData() = default;

                inline int16_t& operator[](size_t index) {
                    return accumulator[index];
                }

                inline const int16_t& operator[](size_t index) const {
                    return accumulator[index];
                }

                inline int16_t* data() {
                    return accumulator;
                }

                inline const int16_t* data() const {
                    return accumulator;
                }

                inline int16_t* begin() {
                    return accumulator;
                }

                inline const int16_t* begin() const {
                    return accumulator;
                }

                inline int16_t* end() {
                    return accumulator + Network::REN_SIZE;
                }

                inline const int16_t* end() const {
                    return accumulator + Network::REN_SIZE;
                }
            };

            const Network& network;
            Accumulator accumulator;
            std::vector<AccumulatorData> pastAccumulators;

            void initializeFromBoard(const Board& board, int32_t color) noexcept;
            void updateAfterOppKingMove(const Board& board, int32_t color, Move move) noexcept;

        public:
            Instance(const Network& net) noexcept;
            Instance() noexcept : Instance(DEFAULT_NETWORK) {}
            ~Instance() noexcept;

            int evaluate(int color, size_t iter = 2) const noexcept;
            void initializeFromBoard(const Board& board) noexcept;
            void updateAfterMove(const Board& board) noexcept;
            void undoMove() noexcept;

            inline void clearPastAccumulators() noexcept {
                pastAccumulators.clear();
            }

    };
}

#endif