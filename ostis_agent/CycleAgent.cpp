#include "sc-agents-common/utils/AgentUtils.hpp"
#include "CycleAgent.hpp"

using namespace std;
using namespace utils;

namespace CycleModule
{

    ScAddr get_set_vertexes(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){

        ScAddr rrel_nodes, empty;
        rrel_nodes = context->HelperResolveSystemIdtf("rrel_nodes");

        ScIterator5Ptr vertexes = context->Iterator5(
                graph,
                ScType::EdgeAccessConstPosPerm,
                ScType(0),
                ScType::EdgeAccessConstPosPerm,
                rrel_nodes
        );

        if (vertexes->Next())
            return vertexes->Get(2);
        else
            return empty;
    }
    ScAddr get_set_arcs(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){

        ScAddr rrel_arcs, empty;
        rrel_arcs = context->HelperResolveSystemIdtf("rrel_arcs");

        ScIterator5Ptr arcs_it = context->Iterator5(
                graph,
                ScType::EdgeAccessConstPosPerm,
                ScType(0),
                ScType::EdgeAccessConstPosPerm,
                rrel_arcs
        );

        if (arcs_it->Next())
            return arcs_it->Get(2);
        else
            return empty;
    }

    

    bool is_used(const std::unique_ptr<ScMemoryContext>& context, ScAddr used, ScAddr element){

        ScIterator3Ptr used_it = context->Iterator3(
                used,
                ScType::EdgeAccessConstPosPerm,
                ScType::Unknown
                );

        while (used_it->Next())
            if(used_it->Get(2) == element){
                SC_LOG_INFO(context->HelperGetSystemIdtf(element) + " is already used");
                return true;
            }
        return false;
    }

    void dfs(const std::unique_ptr<ScMemoryContext>& context, ScAddr &used, ScAddr curr, ScAddr &arcs_set){

        SC_LOG_INFO("Now in " + context->HelperGetSystemIdtf(curr));

        context->CreateEdge(ScType::EdgeAccessConstPosPerm, used, curr);

        ScIterator5Ptr curr_it = context->Iterator5(
                curr,
                ScType::EdgeAccessConstPosPerm,
                ScType(0),
                ScType::EdgeAccessConstPosPerm,
                arcs_set
        );

        while (curr_it->Next())
            if(!is_used(context, used, curr_it->Get(2)))
                dfs(context, used, curr_it->Get(2), arcs_set);

        SC_LOG_INFO("Quit of " + context->HelperGetSystemIdtf(curr));
    }

    bool do_dfs(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph, ScAddr arcs_set){

        ScAddr vertexes_set = get_set_vertexes(context, graph);
        if(vertexes_set.Hash() == 0){
            SC_LOG_INFO("Error! Graph has no vertices");
            return false;
        }

        ScAddr start;
        ScIterator3Ptr v = context->Iterator3(
                vertexes_set,
                ScType::EdgeAccessConstPosPerm,
                ScType::Unknown
        );
        if (v->Next())
            start = v->Get(2);
        else
            start = context->CreateNode(ScType::Const);

        ScAddr used = context->CreateNode(ScType::Const);

        SC_LOG_INFO("-----Start-----");
        dfs(context, used, start, arcs_set);
        SC_LOG_INFO("------End------");

        ScIterator3Ptr vertex_it = context->Iterator3(
                    vertexes_set,
                    ScType::EdgeAccessConstPosPerm,
                    ScType::Unknown
        );

        bool present;

        while (vertex_it->Next()){
            ScIterator3Ptr used_it = context->Iterator3(
                used,
                ScType::EdgeAccessConstPosPerm,
                ScType::Unknown
            );
            present = false;
            while(used_it->Next()){
                if(vertex_it->Get(2) == used_it->Get(2))
                    present = true;
            }
            if(!present){
                SC_LOG_INFO("Not all vertexes have been visited!");
                return false;
            }
        }
        SC_LOG_INFO("All vertexes have been visited!");
        return true;
    }

    void check_graph(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){
        ScAddr strongly_connected_set = context->CreateNode(ScType::NodeConstClass);
        context->HelperSetSystemIdtf("find cycle in graph", strongly_connected_set);

        SC_LOG_INFO("~~~~~~~~" + context->HelperGetSystemIdtf(graph) + "~~~~~~~~");

        if(do_dfs(context, graph, get_set_arcs(context, graph)))
        {
            context->CreateEdge(ScType::EdgeAccessConstPosPerm, strongly_connected_set, graph);
            SC_LOG_INFO("Result: TRUE");
        }
        else {
            context->CreateEdge(ScType::EdgeAccessConstNegPerm, strongly_connected_set, graph);
            SC_LOG_INFO("Result: FALSE");
        }
        SC_LOG_INFO("~~~~~~~~" + context->HelperGetSystemIdtf(graph) + "~~~~~~~~");
    }

    void run_test(const std::unique_ptr<ScMemoryContext>& context){
        ScAddr graph_set = context->HelperResolveSystemIdtf("graph");
        ScIterator3Ptr graph_it = context->Iterator3(
                graph_set,
                ScType::EdgeAccessConstPosPerm,
                ScType::Unknown
        );
        while(graph_it->Next()){
            check_graph(context, graph_it->Get(2));
            SC_LOG_INFO(" ");
        }
    }

    SC_AGENT_IMPLEMENTATION(CycleAgent)
    {
        SC_LOG_INFO("Cycle graph finder was activated");

        if (!edgeAddr.IsValid())
            return SC_RESULT_ERROR;

        //to test curr graph
        ScAddr question = ms_context->GetEdgeTarget(edgeAddr);
        check_graph(ms_context, question);

        //to run all tests
        //run_test(ms_context);

        return SC_RESULT_OK;
    }
} // namespace exampleModule

