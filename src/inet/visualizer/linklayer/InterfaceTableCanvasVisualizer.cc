//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/visualizer/linklayer/InterfaceTableCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(InterfaceTableCanvasVisualizer);

InterfaceTableCanvasVisualizer::InterfaceCanvasVisualization::InterfaceCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int networkNodeId, int networkNodeGateId, int interfaceId) :
    InterfaceVisualization(networkNodeId, networkNodeGateId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

InterfaceTableCanvasVisualizer::InterfaceCanvasVisualization::~InterfaceCanvasVisualization()
{
    delete figure;
}

void InterfaceTableCanvasVisualizer::initialize(int stage)
{
    InterfaceTableVisualizerBase::initialize(stage);
    if (!hasGUI())
        return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

InterfaceTableVisualizerBase::InterfaceVisualization *InterfaceTableCanvasVisualizer::createInterfaceVisualization(cModule *networkNode, NetworkInterface *networkInterface)
{
    BoxedLabelFigure *figure = nullptr;
    auto gate = getOutputGate(networkNode, networkInterface);
    if (!displayWiredInterfacesAtConnections || gate == nullptr) {
        figure = new BoxedLabelFigure("networkInterface");
        figure->setTags((std::string("network_interface ") + tags).c_str());
        figure->setTooltip("This label represents a network interface in a network node");
        figure->setAssociatedObject(networkInterface);
        figure->setZIndex(zIndex);
        figure->setFont(font);
        figure->setText(getVisualizationText(networkInterface).c_str());
        figure->setLabelColor(textColor);
        figure->setBackgroundColor(backgroundColor);
        figure->setOpacity(opacity);
        if (!displayBackground) {
            figure->setInset(0);
            figure->getRectangleFigure()->setVisible(false);
        }
    }
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create interface visualization for '%s', because network node visualization is not found for '%s'", networkInterface->getInterfaceName(), networkNode->getFullPath().c_str());
    return new InterfaceCanvasVisualization(networkNodeVisualization, figure, networkNode->getId(), gate == nullptr ? -1 : gate->getId(), networkInterface->getInterfaceId());
}

NetworkInterface *InterfaceTableCanvasVisualizer::getNetworkInterface(const InterfaceVisualization *interfaceVisualization)
{
    L3AddressResolver addressResolver;
    auto networkNode = getNetworkNode(interfaceVisualization);
    if (networkNode == nullptr)
        return nullptr;
    auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
    if (interfaceTable == nullptr)
        return nullptr;
    return interfaceTable->getInterfaceById(interfaceVisualization->interfaceId);
}

void InterfaceTableCanvasVisualizer::addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::addInterfaceVisualization(interfaceVisualization);
    auto interfaceCanvasVisualization = static_cast<const InterfaceCanvasVisualization *>(interfaceVisualization);
    if (interfaceCanvasVisualization->figure == nullptr) {
        auto gate = getOutputGate(interfaceVisualization);
        if (gate != nullptr && gate->getChannel()) {
            cDisplayString& displayString = gate->getDisplayString();
            displayString.setTagArg("t", 0, getVisualizationText(getNetworkInterface(interfaceVisualization)).c_str());
            displayString.setTagArg("t", 1, "l");
        }
    }
    else
        interfaceCanvasVisualization->networkNodeVisualization->addAnnotation(interfaceCanvasVisualization->figure, interfaceCanvasVisualization->figure->getBounds().getSize(), placementHint, placementPriority);
}

void InterfaceTableCanvasVisualizer::removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::removeInterfaceVisualization(interfaceVisualization);
    auto interfaceCanvasVisualization = static_cast<const InterfaceCanvasVisualization *>(interfaceVisualization);
    if (interfaceCanvasVisualization->figure == nullptr) {
        auto gate = getOutputGate(interfaceVisualization);
        if (gate != nullptr && gate->getChannel())
            gate->getDisplayString().setTagArg("t", 0, "");
    }
    else if (networkNodeVisualizer != nullptr)
        interfaceCanvasVisualization->networkNodeVisualization->removeAnnotation(interfaceCanvasVisualization->figure);
}

void InterfaceTableCanvasVisualizer::refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const NetworkInterface *networkInterface)
{
    auto interfaceCanvasVisualization = static_cast<const InterfaceCanvasVisualization *>(interfaceVisualization);
    auto figure = interfaceCanvasVisualization->figure;
    if (figure == nullptr) {
        auto gate = getOutputGate(interfaceVisualization);
        if (gate != nullptr && gate->getChannel()) {
            cDisplayString& displayString = gate->getDisplayString();
            displayString.setTagArg("t", 0, getVisualizationText(networkInterface).c_str());
            displayString.setTagArg("t", 1, "l");
        }
    }
    else {
        figure->setText(getVisualizationText(networkInterface).c_str());
        interfaceCanvasVisualization->networkNodeVisualization->setAnnotationSize(figure, figure->getBounds().getSize());
    }
}

} // namespace visualizer

} // namespace inet

