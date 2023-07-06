#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "CycleAgent.generated.hpp"

namespace CycleModule
{
    class CycleAgent : public ScAgent
    {
        SC_CLASS(Agent, Event(Keynodes::question_is_cycle, ScEvent::Type::AddOutputEdge))
        SC_GENERATED_BODY()
    };

} // namespace exampleModule
