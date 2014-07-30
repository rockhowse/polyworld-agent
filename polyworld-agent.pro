#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T02:10:40
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

# needed for std::function in SheetsModel.h
QMAKE_CC = g++ -std=gnu++0x

TARGET = polyworld-agent
TEMPLATE = app

LIBS += -rdynamic -pthread -ldl -lz -lgsl -lgslcblas -lGL -lGLU -lpython2.7

INCLUDEPATH += . \
               agent \
               app \
               brain \
               brain/groups \
               brain/sheets \
               complexity \
               utils \
               proplib \
               environment \
               graphics \
               genome \
               genome/groups \
               genome/sheets \
#               logs \
               network \
               /usr/include/python2.7/

SOURCES += main.cpp\
        polyworldagent.cpp \
    network/NetworkClient.cpp \
    agent/agent.cp \
    agent/AgentAttachedData.cp \
    agent/AgentPovRenderer.cp \
    agent/BeingCarriedSensor.cp \
    agent/CarryingSensor.cp \
    agent/EnergySensor.cp \
    agent/LifeSpan.cp \
    agent/MateWaitSensor.cp \
    agent/Metabolism.cp \
    agent/RandomSensor.cp \
    agent/Retina.cp \
    agent/SpeedSensor.cp \
    brain/Brain.cp \
    brain/FiringRateModel.cp \
    brain/Nerve.cp \
    brain/NervousSystem.cp \
    brain/SpikingModel.cp \
    brain/groups/GroupsBrain.cp \
    brain/sheets/SheetsBrain.cp \
    brain/sheets/SheetsModel.cp \
    proplib/builder.cp \
    proplib/dom.cp \
    proplib/editor.cp \
    proplib/overlay.cp \
    proplib/schema.cp \
    proplib/writer.cp \
    proplib/interpreter.cp \
    proplib/convert.cp \
    proplib/cppprops.cp \
    proplib/expression.cp \
    proplib/parser.cp \
    proplib/state.cp \
    utils/AbstractFile.cp \
    utils/datalib.cp \
    utils/distributions.cp \
    utils/error.cp \
    utils/indexlist.cp \
    utils/misc.cp \
    utils/Mutex.cp \
    utils/objectxsortedlist.cp \
    utils/PwMovieUtils.cp \
    utils/RandomNumberGenerator.cp \
    utils/Resources.cp \
    utils/Scalar.cp \
    utils/Variant.cp \
    app/debug.cp \
    environment/Energy.cp \
    graphics/gcamera.cp \
    graphics/gsquare.cp \
    graphics/gobject.cp \
    graphics/graphics.cp \
    genome/Genome.cp \
    app/globals.cp \
    graphics/gmisc.cp \
    environment/brick.cp \
    environment/food.cp \
    genome/groups/GroupsGenome.cp \
    app/simtypes.cp \
    genome/sheets/SheetsCrossover.cp \
    genome/sheets/SheetsGenome.cp \
    genome/sheets/SheetsGenomeSchema.cp \
    graphics/glight.cp \
    complexity/adami.cp \
    environment/barrier.cp \
    environment/BrickPatch.cp \
    environment/FoodPatch.cp \
    environment/FoodType.cp \
    environment/Patch.cp \
    app/FittestList.cp \
    genome/groups/GroupsGene.cp \
    genome/groups/GroupsGenomeSchema.cp \
    genome/groups/GroupsSynapseType.cp \
    genome/Gene.cp \
    genome/GeneSchema.cp \
    genome/GenomeLayout.cp \
    genome/GenomeSchema.cp \
    genome/GenomeUtil.cp \
    genome/SeparationCache.cp \
    graphics/gpolygon.cp \
    graphics/gscene.cp \
    graphics/gstage.cp \
    network/MulticastReceiver.cpp

HEADERS  += polyworldagent.h \
    network/NetworkClient.h \
    agent/agent.h \
    agent/AgentAttachedData.h \
    agent/AgentListener.h \
    agent/AgentPovRenderer.h \
    agent/BeingCarriedSensor.h \
    agent/CarryingSensor.h \
    agent/EnergySensor.h \
    agent/LifeSpan.h \
    agent/MateWaitSensor.h \
    agent/Metabolism.h \
    agent/RandomSensor.h \
    agent/Retina.h \
    agent/SpeedSensor.h \
    brain/BaseNeuronModel.h \
    brain/Brain.h \
    brain/FiringRateModel.h \
    brain/Nerve.h \
    brain/NervousSystem.h \
    brain/NeuralNetRenderer.h \
    brain/NeuronModel.h \
    brain/Sensor.h \
    brain/SpikingModel.h \
    brain/groups/GroupsBrain.h \
    brain/groups/GroupsNeuralNetRenderer.h \
    brain/sheets/SheetsBrain.h \
    brain/sheets/SheetsModel.h \
    utils/objectlist.h \
    proplib/proplib.h \
    proplib/builder.h \
    proplib/dom.h \
    proplib/editor.h \
    proplib/overlay.h \
    proplib/schema.h \
    proplib/writer.h \
    proplib/interpreter.h \
    proplib/convert.h \
    proplib/cppprops.h \
    proplib/expression.h \
    proplib/parser.h \
    proplib/state.h \
    utils/AbstractFile.h \
    utils/datalib.h \
    utils/distributions.h \
    utils/error.h \
    utils/Events.h \
    utils/gdlink.h \
    utils/graybin.h \
    utils/indexlist.h \
    utils/misc.h \
    utils/Mutex.h \
    utils/next_combination.h \
    utils/objectxsortedlist.h \
    utils/PwMovieUtils.h \
    utils/Queue.h \
    utils/RandomNumberGenerator.h \
    utils/Resources.h \
    utils/Scalar.h \
    utils/Variant.h \
    app/debug.h \
    environment/Energy.h \
    graphics/gcamera.h \
    graphics/gsquare.h \
    graphics/gobject.h \
    graphics/graphics.h \
    genome/Genome.h \
    app/globals.h \
    graphics/gmisc.h \
    environment/brick.h \
    environment/food.h \
    genome/groups/GroupsGenome.h \
    app/simtypes.h \
    genome/sheets/SheetsCrossover.h \
    genome/sheets/SheetsGenome.h \
    genome/sheets/SheetsGenomeSchema.h \
    graphics/glight.h \
    complexity/adami.h \
    environment/barrier.h \
    environment/BrickPatch.h \
    environment/FoodPatch.h \
    environment/FoodType.h \
    environment/Patch.h \
    app/Domain.h \
    app/FittestList.h \
    genome/groups/GroupsGene.h \
    genome/groups/GroupsGenomeSchema.h \
    genome/groups/GroupsSynapseType.h \
    genome/Gene.h \
    genome/GeneSchema.h \
    genome/GenomeLayout.h \
    genome/GenomeSchema.h \
    genome/GenomeUtil.h \
    genome/NeurGroupType.h \
    genome/NeuronType.h \
    genome/SeparationCache.h \
    graphics/gpolygon.h \
    graphics/gscene.h \
    graphics/gstage.h \
    network/MulticastReceiver.h

FORMS    += polyworldagent.ui

OTHER_FILES += \
    README.md
