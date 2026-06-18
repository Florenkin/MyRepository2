#include "DlpProjectorScanner.h"

#include <chrono>
#include <cstdio>
#include <limits>
#include <thread>

DlpProjectorScanner* DlpProjectorScanner::activeInstance_ = nullptr;
std::vector<uint8_t>* DlpProjectorScanner::activePatternDataBlock_ = nullptr;

DlpProjectorScanner::DlpProjectorScanner(const DlpDeviceConfig& deviceConfig)
    : deviceConfig_(deviceConfig)
{
}

DlpProjectorScanner::~DlpProjectorScanner()
{
    disconnectProjector();

    if (activeInstance_ == this)
    {
        activeInstance_ = nullptr;
    }
}

void DlpProjectorScanner::setDeviceConfig(const DlpDeviceConfig& deviceConfig)
{
    deviceConfig_ = deviceConfig;
}

DlpDeviceConfig DlpProjectorScanner::deviceConfig() const
{
    return deviceConfig_;
}

bool DlpProjectorScanner::isConnected() const
{
    return transport_.isOpen();
}

bool DlpProjectorScanner::connectProjector()
{
    if (transport_.isOpen())
    {
        return true;
    }

    bool ok = transport_.open(
        deviceConfig_.cypressVid,
        deviceConfig_.cypressPid,
        deviceConfig_.dlpcI2cAddress
    );

    if (!ok)
    {
        setLastError(transport_.lastError());
        printf("[DLP USB ERROR] %s\n", lastError_.c_str());
        return false;
    }

    setLastError("");
    printf("[DLP USB OK] Projector connected\n");
    return true;
}

void DlpProjectorScanner::disconnectProjector()
{
    if (transport_.isOpen())
    {
        transport_.close();
        printf("[DLP USB OK] Projector disconnected\n");
    }
}

bool DlpProjectorScanner::initializeCommandLibrary()
{
    activeInstance_ = this;

    DLPC_COMMON_InitCommandLibrary(
        writeBuffer_,
        sizeof(writeBuffer_),
        readBuffer_,
        sizeof(readBuffer_),
        &DlpProjectorScanner::writeCommandCallback,
        &DlpProjectorScanner::readCommandCallback
    );

    setLastError("");
    printf("[DLP OK] DLPC command library initialized\n");
    return true;
}

bool DlpProjectorScanner::validateScanConfig(const DlpScanConfig& config)
{
    if (config.internalPatterns.empty())
    {
        setLastError("internalPatterns is empty");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    if (config.internalPatterns.size() > 255)
    {
        setLastError("internalPatterns count must not exceed 255");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    if (config.numberOfPatternsInSet == 0)
    {
        setLastError("numberOfPatternsInSet must be greater than 0");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    for (size_t i = 0; i < config.internalPatterns.size(); ++i)
    {
        const DlpInternalPatternConfig& pattern = config.internalPatterns[i];

        if (pattern.numberOfPatternsToDisplay == 0)
        {
            setLastError("numberOfPatternsToDisplay must be greater than 0");
            printf("[DLP ERROR] internalPatterns[%zu]: %s\n", i, lastError_.c_str());
            return false;
        }

        if ((pattern.redLed == DLPC34XX_IE_DISABLE) &&
            (pattern.greenLed == DLPC34XX_IE_DISABLE) &&
            (pattern.blueLed == DLPC34XX_IE_DISABLE))
        {
            setLastError("at least one LED must be enabled");
            printf("[DLP ERROR] internalPatterns[%zu]: %s\n", i, lastError_.c_str());
            return false;
        }
    }

    setLastError("");
    return true;
}

bool DlpProjectorScanner::configureScan(const DlpScanConfig& config)
{
    if (!validateScanConfig(config))
    {
        return false;
    }

    uint32_t status = 0;

    if (config.writePatternConfiguration)
    {
        const DlpInternalPatternConfig& firstPattern = config.internalPatterns[0];

        DLPC34XX_PatternConfiguration_s patternConfiguration = {};
        patternConfiguration.SequenceType = config.sequenceType;
        patternConfiguration.NumberOfPatterns = config.numberOfPatternsInSet;
        patternConfiguration.RedIlluminator = firstPattern.redLed;
        patternConfiguration.GreenIlluminator = firstPattern.greenLed;
        patternConfiguration.BlueIlluminator = firstPattern.blueLed;
        patternConfiguration.IlluminationTime = maxIlluminationTimeFromConfig(config);
        patternConfiguration.PreIlluminationDarkTime = maxPreDarkTimeFromConfig(config);
        patternConfiguration.PostIlluminationDarkTime = maxPostDarkTimeFromConfig(config);

        status = DLPC34XX_WritePatternConfiguration(&patternConfiguration);

        if (!checkStatus(status, "Write pattern configuration"))
        {
            return false;
        }
    }

    if (config.writeInternalPatternDisplayConfiguration)
    {
        status = DLPC34XX_WriteInternalPatternDisplayConfiguration(
            config.dmdBlockStart,
            config.dmdBlockCount
        );

        if (!checkStatus(status, "Write internal pattern display configuration"))
        {
            return false;
        }
    }

    if (config.writeTriggerOut1)
    {
        status = DLPC34XX_WriteTriggerOutConfiguration(
            DLPC34XX_TT_TRIGGER1,
            DLPC34XX_TE_ENABLE,
            config.triggerOut1Inversion,
            config.triggerOut1Delay
        );

        if (!checkStatus(status, "Write trigger out 1 configuration"))
        {
            return false;
        }
    }

    if (config.writeTriggerOut2)
    {
        status = DLPC34XX_WriteTriggerOutConfiguration(
            DLPC34XX_TT_TRIGGER2,
            DLPC34XX_TE_ENABLE,
            config.triggerOut2Inversion,
            config.triggerOut2Delay
        );

        if (!checkStatus(status, "Write trigger out 2 configuration"))
        {
            return false;
        }
    }

    if (config.writePatternOrderTable)
    {
        for (size_t i = 0; i < config.internalPatterns.size(); ++i)
        {
            const DlpInternalPatternConfig& pattern = config.internalPatterns[i];

            DLPC34XX_PatternOrderTableEntry_s orderEntry = {};
            orderEntry.PatSetIndex = pattern.patternSetIndex;
            orderEntry.NumberOfPatternsToDisplay = pattern.numberOfPatternsToDisplay;
            orderEntry.RedIlluminator = pattern.redLed;
            orderEntry.GreenIlluminator = pattern.greenLed;
            orderEntry.BlueIlluminator = pattern.blueLed;
            orderEntry.PatternInvertLsword = patternInvertMask(pattern.invertPatterns);
            orderEntry.PatternInvertMsword = patternInvertMask(pattern.invertPatterns);
            orderEntry.IlluminationTime = pattern.illuminationTimeUs;
            orderEntry.PreIlluminationDarkTime = pattern.preIlluminationDarkTimeUs;
            orderEntry.PostIlluminationDarkTime = pattern.postIlluminationDarkTimeUs;
            orderEntry.PatternEntryIndex = pattern.patternEntryIndex;

            status = DLPC34XX_WritePatternOrderTableEntry(
                (i == 0) ? DLPC34XX_WC_START : DLPC34XX_WC_CONTINUE,
                &orderEntry
            );

            if (!checkStatus(status, "Write pattern order table entry"))
            {
                return false;
            }
        }
    }

    return true;
}

bool DlpProjectorScanner::prepareScan(const DlpScanConfig& config)
{
    uint32_t status = DLPC34XX_WriteInternalPatternControl(
        DLPC34XX_PC_STOP,
        0
    );

    checkStatus(status, "Stop previous internal pattern");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    status = DLPC34XX_WriteInternalPatternControl(
        DLPC34XX_PC_RESET,
        0
    );

    if (!checkStatus(status, "Reset internal pattern"))
    {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (!configureScan(config))
    {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    status = DLPC34XX_WriteOperatingModeSelect(
        DLPC34XX_OM_SENS_INTERNAL_PATTERN
    );

    if (!checkStatus(status, "Set internal pattern mode"))
    {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    DLPC34XX_InternalPatternStatus_s patternStatus = {};
    status = DLPC34XX_ReadInternalPatternStatus(&patternStatus);

    if (!checkStatus(status, "Read internal pattern status"))
    {
        return false;
    }

    printInternalPatternStatus(patternStatus);

    if (patternStatus.PatternReadyStatus != DLPC34XX_PRS_READY)
    {
        setLastError("Pattern not ready. Flash may not contain pattern data.");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    printf("[DLP OK] Pattern ready\n");
    return true;
}

bool DlpProjectorScanner::startScan(const DlpScanConfig& config)
{
    uint32_t status = DLPC34XX_WriteInternalPatternControl(
        DLPC34XX_PC_START,
        config.repeatCount
    );

    return checkStatus(status, "Start internal pattern");
}

bool DlpProjectorScanner::stopScan()
{
    uint32_t status = DLPC34XX_WriteInternalPatternControl(
        DLPC34XX_PC_STOP,
        0
    );

    return checkStatus(status, "Stop internal pattern");
}

bool DlpProjectorScanner::runScanFor(const DlpScanConfig& config, uint32_t playTimeMs)
{
    if (!connectProjector())
    {
        return false;
    }

    initializeCommandLibrary();

    if (!prepareScan(config))
    {
        return false;
    }

    if (!startScan(config))
    {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(playTimeMs));

    return stopScan();
}

std::string DlpProjectorScanner::lastError() const
{
    return lastError_;
}

bool DlpProjectorScanner::generateInternalPatternDataBlock(
    const DlpPatternDataBlockConfig& config,
    std::vector<uint8_t>* patternDataBlock)
{
    if (patternDataBlock == nullptr)
    {
        setLastError("patternDataBlock output pointer is null");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    patternDataBlock->clear();

    if (config.patternSets.empty())
    {
        setLastError("patternSets is empty");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    if (config.patternOrder.empty())
    {
        setLastError("patternOrder is empty");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    std::vector<std::vector<DLPC34XX_INT_PAT_PatternData_s>> patternDataStorage;
    std::vector<DLPC34XX_INT_PAT_PatternSet_s> patternSetArray;
    std::vector<DLPC34XX_INT_PAT_PatternOrderTableEntry_s> patternOrderTable;

    patternDataStorage.resize(config.patternSets.size());
    patternSetArray.resize(config.patternSets.size());

    for (size_t setIndex = 0; setIndex < config.patternSets.size(); ++setIndex)
    {
        const DlpPatternSetData& sourceSet = config.patternSets[setIndex];

        if (sourceSet.patterns.empty())
        {
            setLastError("pattern set has no patterns");
            printf("[DLP ERROR] patternSets[%zu]: %s\n", setIndex, lastError_.c_str());
            return false;
        }

        patternDataStorage[setIndex].resize(sourceSet.patterns.size());

        for (size_t patternIndex = 0; patternIndex < sourceSet.patterns.size(); ++patternIndex)
        {
            const DlpPatternImage& sourcePattern = sourceSet.patterns[patternIndex];

            if (sourcePattern.pixels.empty())
            {
                setLastError("pattern pixels is empty");
                printf("[DLP ERROR] patternSets[%zu].patterns[%zu]: %s\n",
                    setIndex,
                    patternIndex,
                    lastError_.c_str());
                return false;
            }

            if (sourcePattern.pixels.size() > std::numeric_limits<uint32_t>::max())
            {
                setLastError("pattern pixels is too large");
                printf("[DLP ERROR] patternSets[%zu].patterns[%zu]: %s\n",
                    setIndex,
                    patternIndex,
                    lastError_.c_str());
                return false;
            }

            patternDataStorage[setIndex][patternIndex].PixelArrayCount =
                static_cast<uint32_t>(sourcePattern.pixels.size());
            patternDataStorage[setIndex][patternIndex].PixelArray =
                const_cast<uint8_t*>(sourcePattern.pixels.data());
        }

        if (sourceSet.patterns.size() > std::numeric_limits<uint32_t>::max())
        {
            setLastError("pattern count is too large");
            printf("[DLP ERROR] patternSets[%zu]: %s\n", setIndex, lastError_.c_str());
            return false;
        }

        patternSetArray[setIndex].BitDepth = sourceSet.depth;
        patternSetArray[setIndex].Direction = sourceSet.direction;
        patternSetArray[setIndex].PatternCount = static_cast<uint32_t>(sourceSet.patterns.size());
        patternSetArray[setIndex].PatternArray = patternDataStorage[setIndex].data();
    }

    patternOrderTable.resize(config.patternOrder.size());

    for (size_t orderIndex = 0; orderIndex < config.patternOrder.size(); ++orderIndex)
    {
        const DlpInternalPatternConfig& sourceOrder = config.patternOrder[orderIndex];

        patternOrderTable[orderIndex].PatternSetIndex = sourceOrder.patternSetIndex;
        patternOrderTable[orderIndex].NumDisplayPatterns = sourceOrder.numberOfPatternsToDisplay;
        patternOrderTable[orderIndex].IlluminationSelect =
            illuminationFromPatternConfig(sourceOrder);
        patternOrderTable[orderIndex].InvertPatterns = sourceOrder.invertPatterns;
        patternOrderTable[orderIndex].IlluminationTimeInMicroseconds =
            sourceOrder.illuminationTimeUs;
        patternOrderTable[orderIndex].PreIlluminationDarkTimeInMicroseconds =
            sourceOrder.preIlluminationDarkTimeUs;
        patternOrderTable[orderIndex].PostIlluminationDarkTimeInMicroseconds =
            sourceOrder.postIlluminationDarkTimeUs;
        patternOrderTable[orderIndex].PatternEntryIndex = sourceOrder.patternEntryIndex;
    }

    uint32_t blockSize = DLPC34XX_INT_PAT_GetPatternDataBlockSize(
        config.dmd,
        static_cast<uint32_t>(patternSetArray.size()),
        patternSetArray.data(),
        static_cast<uint32_t>(patternOrderTable.size()),
        patternOrderTable.data()
    );

    if (blockSize == UINT32_MAX)
    {
        setLastError("unsupported DMD for internal pattern data block");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    patternDataBlock->reserve(blockSize);
    activePatternDataBlock_ = patternDataBlock;

    uint32_t status = DLPC34XX_INT_PAT_GeneratePatternDataBlock(
        config.dmd,
        static_cast<uint32_t>(patternSetArray.size()),
        patternSetArray.data(),
        static_cast<uint32_t>(patternOrderTable.size()),
        patternOrderTable.data(),
        &DlpProjectorScanner::collectPatternDataCallback,
        config.eastWestFlip,
        config.longAxisFlip
    );

    activePatternDataBlock_ = nullptr;

    if (status != DLPC_SUCCESS)
    {
        char errorBuffer[160] = {};
        std::snprintf(
            errorBuffer,
            sizeof(errorBuffer),
            "Generate internal pattern data block failed, status = %u",
            status
        );
        setLastError(errorBuffer);
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        patternDataBlock->clear();
        return false;
    }

    setLastError("");
    printf("[DLP OK] Generate internal pattern data block, bytes = %zu\n",
        patternDataBlock->size());
    return true;
}

bool DlpProjectorScanner::writeInternalPatternDataToFlash(
    const std::vector<uint8_t>& patternDataBlock,
    uint16_t chunkSize)
{
    if (patternDataBlock.empty())
    {
        setLastError("patternDataBlock is empty");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    if (chunkSize == 0)
    {
        setLastError("chunkSize must be greater than 0");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    if (patternDataBlock.size() > std::numeric_limits<uint32_t>::max())
    {
        setLastError("patternDataBlock is too large");
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    uint32_t status = DLPC34XX_WriteFlashDataTypeSelect(
        DLPC34XX_FDTS_ENTIRE_SENS_PATTERN_DATA
    );

    if (!checkStatus(status, "Select sensing pattern flash data"))
    {
        return false;
    }

    DLPC34XX_Error_e packageSizeStatus = DLPC34XX_E_NO_ERROR;
    DLPC34XX_Error_e packageConfigurationCollapsed = DLPC34XX_E_NO_ERROR;
    DLPC34XX_Error_e packageConfigurationIdentifier = DLPC34XX_E_NO_ERROR;

    status = DLPC34XX_ReadFlashUpdatePrecheck(
        static_cast<uint32_t>(patternDataBlock.size()),
        &packageSizeStatus,
        &packageConfigurationCollapsed,
        &packageConfigurationIdentifier
    );

    if (!checkStatus(status, "Flash update precheck"))
    {
        return false;
    }

    if ((packageSizeStatus != DLPC34XX_E_NO_ERROR) ||
        (packageConfigurationCollapsed != DLPC34XX_E_NO_ERROR) ||
        (packageConfigurationIdentifier != DLPC34XX_E_NO_ERROR))
    {
        char errorBuffer[192] = {};
        std::snprintf(
            errorBuffer,
            sizeof(errorBuffer),
            "Flash precheck rejected data, size=%d collapsed=%d identifier=%d",
            static_cast<int>(packageSizeStatus),
            static_cast<int>(packageConfigurationCollapsed),
            static_cast<int>(packageConfigurationIdentifier)
        );
        setLastError(errorBuffer);
        printf("[DLP ERROR] %s\n", lastError_.c_str());
        return false;
    }

    status = DLPC34XX_WriteFlashErase();

    if (!checkStatus(status, "Erase sensing pattern flash data"))
    {
        return false;
    }

    const uint8_t* data = patternDataBlock.data();
    size_t bytesRemaining = patternDataBlock.size();
    size_t offset = 0;
    bool firstChunk = true;

    while (bytesRemaining > 0)
    {
        uint16_t currentChunkSize =
            static_cast<uint16_t>(bytesRemaining < chunkSize ? bytesRemaining : chunkSize);

        status = DLPC34XX_WriteFlashDataLength(currentChunkSize);

        if (!checkStatus(status, "Write flash data length"))
        {
            return false;
        }

        uint8_t* chunkData = const_cast<uint8_t*>(data + offset);

        if (firstChunk)
        {
            status = DLPC34XX_WriteFlashStart(currentChunkSize, chunkData);
            firstChunk = false;
        }
        else
        {
            status = DLPC34XX_WriteFlashContinue(currentChunkSize, chunkData);
        }

        if (!checkStatus(status, "Write sensing pattern flash chunk"))
        {
            return false;
        }

        offset += currentChunkSize;
        bytesRemaining -= currentChunkSize;
    }

    setLastError("");
    printf("[DLP OK] Write internal pattern data to flash, bytes = %zu\n",
        patternDataBlock.size());
    return true;
}

uint32_t DlpProjectorScanner::writeCommandCallback(
    uint16_t writeLength,
    uint8_t* writeBuffer,
    DLPC_COMMON_CommandProtocolData_s* protocolData)
{
    (void)protocolData;

    if (activeInstance_ == nullptr)
    {
        return FAIL;
    }

    return activeInstance_->writeCommand(writeBuffer, writeLength)
        ? DLPC_SUCCESS
        : FAIL;
}

uint32_t DlpProjectorScanner::readCommandCallback(
    uint16_t writeLength,
    uint8_t* writeBuffer,
    uint16_t readLength,
    uint8_t* readBuffer,
    DLPC_COMMON_CommandProtocolData_s* protocolData)
{
    if (activeInstance_ == nullptr)
    {
        return FAIL;
    }

    bool ok = activeInstance_->readCommand(
        writeBuffer,
        writeLength,
        readBuffer,
        readLength
    );

    if (ok && protocolData)
    {
        protocolData->BytesRead = readLength;
    }

    return ok ? DLPC_SUCCESS : FAIL;
}

bool DlpProjectorScanner::writeCommand(uint8_t* data, uint16_t length)
{
    if (!transport_.isOpen())
    {
        setLastError("transport not open");
        printf("[DLP WRITE ERROR] %s\n", lastError_.c_str());
        return false;
    }

    bool ok = transport_.write(data, length);

    if (!ok)
    {
        setLastError(transport_.lastError());
        printf("[DLP WRITE ERROR] %s\n", lastError_.c_str());
        return false;
    }

    setLastError("");
    return true;
}

bool DlpProjectorScanner::readCommand(
    uint8_t* writeData,
    uint16_t writeLength,
    uint8_t* readData,
    uint16_t readLength)
{
    if (!transport_.isOpen())
    {
        setLastError("transport not open");
        printf("[DLP READ ERROR] %s\n", lastError_.c_str());
        return false;
    }

    bool ok = transport_.read(
        writeData,
        writeLength,
        readData,
        readLength
    );

    if (!ok)
    {
        setLastError(transport_.lastError());
        printf("[DLP READ ERROR] %s\n", lastError_.c_str());
        return false;
    }

    setLastError("");
    return true;
}

bool DlpProjectorScanner::checkStatus(uint32_t status, const char* message)
{
    if (status != DLPC_SUCCESS)
    {
        char errorBuffer[160] = {};
        std::snprintf(
            errorBuffer,
            sizeof(errorBuffer),
            "%s failed, status = %u",
            message,
            status
        );

        setLastError(errorBuffer);
        printf("[DLP ERROR] %s\n", lastError_.c_str());

        if (!transport_.lastError().empty())
        {
            printf("[DLP TRANSPORT ERROR] %s\n", transport_.lastError().c_str());
        }

        return false;
    }

    setLastError("");
    printf("[DLP OK] %s\n", message);
    return true;
}

uint32_t DlpProjectorScanner::patternInvertMask(bool invertPatterns)
{
    return invertPatterns ? 0xFFFFFFFFU : 0U;
}

uint32_t DlpProjectorScanner::maxU32(uint32_t left, uint32_t right)
{
    return (left > right) ? left : right;
}

uint32_t DlpProjectorScanner::maxIlluminationTimeFromConfig(const DlpScanConfig& config)
{
    uint32_t maxTimeUs = 0;

    for (const DlpInternalPatternConfig& pattern : config.internalPatterns)
    {
        maxTimeUs = maxU32(maxTimeUs, pattern.illuminationTimeUs);
    }

    return maxTimeUs;
}

uint32_t DlpProjectorScanner::maxPreDarkTimeFromConfig(const DlpScanConfig& config)
{
    uint32_t maxTimeUs = 0;

    for (const DlpInternalPatternConfig& pattern : config.internalPatterns)
    {
        maxTimeUs = maxU32(maxTimeUs, pattern.preIlluminationDarkTimeUs);
    }

    return maxTimeUs;
}

uint32_t DlpProjectorScanner::maxPostDarkTimeFromConfig(const DlpScanConfig& config)
{
    uint32_t maxTimeUs = 0;

    for (const DlpInternalPatternConfig& pattern : config.internalPatterns)
    {
        maxTimeUs = maxU32(maxTimeUs, pattern.postIlluminationDarkTimeUs);
    }

    return maxTimeUs;
}

void DlpProjectorScanner::printInternalPatternStatus(const DLPC34XX_InternalPatternStatus_s& patternStatus)
{
    printf("[DLP INFO] PatternReadyStatus = %d\n",
        static_cast<int>(patternStatus.PatternReadyStatus));

    printf("[DLP INFO] NumPatOrderTableEntries = %d\n",
        static_cast<int>(patternStatus.NumPatOrderTableEntries));

    printf("[DLP INFO] CurrentPatOrderEntryIndex = %d\n",
        static_cast<int>(patternStatus.CurrentPatOrderEntryIndex));

    printf("[DLP INFO] CurrentPatSetIndex = %d\n",
        static_cast<int>(patternStatus.CurrentPatSetIndex));

    printf("[DLP INFO] NumPatInCurrentPatSet = %d\n",
        static_cast<int>(patternStatus.NumPatInCurrentPatSet));

    printf("[DLP INFO] NumPatDisplayedFromPatSet = %d\n",
        static_cast<int>(patternStatus.NumPatDisplayedFromPatSet));
}

void DlpProjectorScanner::collectPatternDataCallback(uint8_t length, uint8_t* data)
{
    if ((activePatternDataBlock_ == nullptr) || (data == nullptr) || (length == 0))
    {
        return;
    }

    activePatternDataBlock_->insert(
        activePatternDataBlock_->end(),
        data,
        data + length
    );
}

DLPC34XX_INT_PAT_IlluminationSelect_e DlpProjectorScanner::illuminationFromPatternConfig(
    const DlpInternalPatternConfig& pattern)
{
    uint8_t illumination = DLPC34XX_INT_PAT_ILLUMINATION_NONE;

    if (pattern.redLed == DLPC34XX_IE_ENABLE)
    {
        illumination |= DLPC34XX_INT_PAT_ILLUMINATION_RED;
    }

    if (pattern.greenLed == DLPC34XX_IE_ENABLE)
    {
        illumination |= DLPC34XX_INT_PAT_ILLUMINATION_GREEN;
    }

    if (pattern.blueLed == DLPC34XX_IE_ENABLE)
    {
        illumination |= DLPC34XX_INT_PAT_ILLUMINATION_BLUE;
    }

    return static_cast<DLPC34XX_INT_PAT_IlluminationSelect_e>(illumination);
}

void DlpProjectorScanner::setLastError(const std::string& errorMessage)
{
    lastError_ = errorMessage;
}
