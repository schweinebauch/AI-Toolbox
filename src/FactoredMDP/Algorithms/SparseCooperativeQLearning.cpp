#include <AIToolbox/FactoredMDP/Algorithms/SparseCooperativeQLearning.hpp>
#include <AIToolbox/FactoredMDP/Utils.hpp>
#include <AIToolbox/FactoredMDP/Algorithms/VariableElimination.hpp>

namespace AIToolbox {
    namespace FactoredMDP {
        SparseCooperativeQLearning::SparseCooperativeQLearning(State s, Action a, double discount, double alpha) :
                S(s), A(a), discount_(discount), alpha_(alpha), rules_(join(S, A)) {}

        void SparseCooperativeQLearning::insertRule(QFunctionRule rule) {
            rules_.emplace(join(S.size(), rule.s_, rule.a_), std::move(rule));
        }

        size_t SparseCooperativeQLearning::rulesSize() const {
            return rules_.size();
        }

        Action SparseCooperativeQLearning::stepUpdateQ(const State & s, const Action & a, const State & s1, const Rewards & rew) {
            VariableElimination ve(a);
            auto a1 = ve(rules_.filter(s1));

            auto beforeRules = rules_.filter(join(s, a));
            auto afterRules = rules_.filter(join(s1, a1.first));

            auto computeQ = [](size_t agent, const decltype(rules_)::Iterable & rules) {
                double sum = 0.0;
                for (const auto & rule : rules)
                    sum += sequential_sorted_contains(rule.a_.first, agent) ? rule.value_ / rule.a_.first.size() : 0.0;
                return sum;
            };

            for (auto & br : beforeRules) {
                double sum = 0;
                for (const auto agent : br.a_.first) {
                    sum += rew[agent];
                    sum += discount_ * computeQ(agent, afterRules);
                    sum -= computeQ(agent, beforeRules);
                }
                br.value_ += alpha_ * sum;
            }
            return a1.first;
        }

        void SparseCooperativeQLearning::setLearningRate(const double a) {
            if ( a <= 0.0 || a > 1.0 ) throw std::invalid_argument("Learning rate parameter must be in (0,1]");
            alpha_ = a;
        }

        double SparseCooperativeQLearning::getLearningRate() const { return alpha_; }

        void SparseCooperativeQLearning::setDiscount(const double d) {
            if ( d <= 0.0 || d > 1.0 ) throw std::invalid_argument("Discount parameter must be in (0,1]");
            discount_ = d;
        }

        double SparseCooperativeQLearning::getDiscount() const { return discount_; }

        const State &  SparseCooperativeQLearning::getS() const { return S; }
        const Action & SparseCooperativeQLearning::getA() const { return A; }
        const FactoredContainer<QFunctionRule> & SparseCooperativeQLearning::getQFunctionRules() const { return rules_; }
    }
}
