#include "opds_controller.hpp"

namespace  adapters::controllers {

OpdsConrtoller::OpdsConrtoller(
    application::IOpdsService* opdsService):
    m_opdsService(opdsService),
    m_opdsModel(&m_opdsService->getOpdsNodes())
{
    connect(
        m_opdsService,&application::IOpdsService::nodesVecReplaceStarted,
        &m_opdsModel,&adapters::data_models::OpdsModel::startedDataChange
        );

         connect(
        m_opdsService,&application::IOpdsService::opdsNodesReady,
        &m_opdsModel,&adapters::data_models::OpdsModel::completedDataChange
        );

}

adapters::data_models::OpdsModel*  OpdsConrtoller::getOpdsModel(){
    return &m_opdsModel;
}

void OpdsConrtoller::loadRootLib(const QString& url){
    if (url.emty())
        return;
    m_opdsService->loadRootLib(url);
}


} //namespace  adapters::controllers