#include "TiffFileSave.h"

/* System */
#include <cstring>
#include <limits>
#include <map>
#include <sstream>

/* libtiff */
#include <tiffio.h>

/* PVCAM */
#include <master.h>
#include <pvcam.h>

/* Local */
#include "Log.h"

pm::TiffFileSave::TiffFileSave(const std::string& fileName, PrdHeader& header)
    : FileSave(fileName, header),
    m_file(nullptr),
    m_frameMeta(nullptr),
    m_frameRecomposed(nullptr),
    m_frameRecomposedBytes(0)
{
    m_frameRecomposedBytes = sizeof(uint16_t) * m_width * m_height;
}

pm::TiffFileSave::~TiffFileSave()
{
    if (IsOpen())
        Close();

    delete [] (uint8_t*)m_frameRecomposed;
    if (m_frameMeta)
        pl_md_release_frame_struct(m_frameMeta);
}

bool pm::TiffFileSave::Open()
{
    if (IsOpen())
        return true;

#if SIZE_MAX > 0xFFFFFFFF
    if (m_rawDataBytes > std::numeric_limits<uint32_t>::max())
    {
        Log::LogE("TIFF format is unable to store more than 4GB raw data");
        return false;
    }
#endif

    m_file = TIFFOpen(m_fileName.c_str(), "w");
    if (!m_file)
        return false;

    m_frameIndex = 0;

    return IsOpen();
}

bool pm::TiffFileSave::IsOpen() const
{
    return !!m_file;
}

void pm::TiffFileSave::Close()
{
    if (m_header.frameCount != m_frameIndex)
    {
        // TODO: Update frame count in TIFF file
        Log::LogE("File does not contain declared number of frame."
            " Automatic correction not implemented yet");

        m_header.frameCount = m_frameIndex;

        //m_file.seekp(0);
        //m_file.write((char*)&m_header, sizeof(PrdHeader));
        //m_file.seekp(0, std::ios_base::end);
    }

    TIFFFlush(m_file);
    TIFFClose(m_file);
    m_file = nullptr;
}

bool pm::TiffFileSave::WriteFrame(const PrdMetaData* metaData, const void* rawData)
{
    if (!FileSave::WriteFrame(metaData, rawData))
        return false;

    TIFFSetField(m_file, TIFFTAG_IMAGEWIDTH, m_width);
    TIFFSetField(m_file, TIFFTAG_IMAGELENGTH, m_height);
    TIFFSetField(m_file, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(m_file, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(m_file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(m_file, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(m_file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(m_file, TIFFTAG_MAXSAMPLEVALUE, (1 << m_header.bitDepth) - 1);

    if (m_header.frameCount > 1)
    {
        // We are writing single page of the multipage file
        TIFFSetField(m_file, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
        // Set the page number
        TIFFSetField(m_file, TIFFTAG_PAGENUMBER, m_frameIndex, m_header.frameCount);
    }

    // Recompose the black-filled frame with metadata if any
    void* tiffData;
    size_t tiffDataBytes;
    if (m_header.version >= PRD_VERSION_0_3
            && (m_header.flags & PRD_FLAG_HAS_METADATA))
    {
        // Allocate md_frame only once
        if (!m_frameMeta)
        {
            if (PV_OK != pl_md_create_frame_struct(&m_frameMeta,
                        (void*)rawData, (uns32)m_rawDataBytes))
                return false;
            if (m_frameRecomposedBytes == 0)
                return false;
            m_frameRecomposed = new(std::nothrow) uint8_t[m_frameRecomposedBytes];
            if (!m_frameRecomposed)
                return false;
        }

        // Fill the frame with black pixels
        std::memset(m_frameRecomposed, 0, m_frameRecomposedBytes);

        // Decode metadata and recompose the frame
        if (PV_OK != pl_md_frame_decode(m_frameMeta, (void *)rawData,
                    (uns32)m_rawDataBytes))
            return false;

        const uint16_t xOff =
            m_frameMeta->impliedRoi.s1 / m_frameMeta->impliedRoi.sbin;
        const uint16_t yOff =
            m_frameMeta->impliedRoi.p1 / m_frameMeta->impliedRoi.pbin;

        if (PV_OK != pl_md_frame_recompose(m_frameRecomposed, xOff, yOff,
                    (uns16)m_width, (uns16)m_height, m_frameMeta))
            return false;

        tiffDataBytes = m_frameRecomposedBytes;
        tiffData = m_frameRecomposed;
    }
    else
    {
        tiffData = (void*)rawData;
        tiffDataBytes = m_rawDataBytes;
    }

    // Build up the metadata description
    const std::string imageDesc = GetImageDesc(m_header, metaData, m_frameMeta);
    // Put the PVCAM metadata into the image description
    TIFFSetField(m_file, TIFFTAG_IMAGEDESCRIPTION, imageDesc.c_str());

    // Skip the metadata at the beginning of the file
    if ((size_t)TIFFWriteRawStrip(m_file, 0, tiffData, tiffDataBytes) == tiffDataBytes)
    {
        if (m_header.frameCount > 1)
            TIFFWriteDirectory(m_file);
    }
    else
    {
        return false;
    }

    m_frameIndex++;
    return true;
}

std::string pm::TiffFileSave::GetImageDesc(const PrdHeader& prdHeader,
        const PrdMetaData* prdMeta, const md_frame* pvcamMeta)
{
    if (!prdMeta)
        return "";

    std::ostringstream imageDesc;

    imageDesc << "bitDepth=" << prdHeader.bitDepth;
    if (prdHeader.version >= PRD_VERSION_0_1)
    {
        static std::map<uint32_t, const char*> expResUnit =
        {
            { PRD_EXP_RES_US, "us" },
            { PRD_EXP_RES_MS, "ms" },
            { PRD_EXP_RES_S, "s" }
        };
        imageDesc
            << "\nregion=[" << prdHeader.region.s1 << "," << prdHeader.region.s2
                << "," << prdHeader.region.sbin << "," << prdHeader.region.p1
                << "," << prdHeader.region.p2 << "," << prdHeader.region.pbin << "]"
            << "\nframeNr=" << prdMeta->frameNumber
            << "\nreadoutTime=" << prdMeta->readoutTime << "us"
            << "\nexpTime=" << prdMeta->exposureTime
                << ((expResUnit.count(prdHeader.exposureResolution) > 0)
                        ? expResUnit[prdHeader.exposureResolution]
                        : "<unknown unit>");
    }
    if (prdHeader.version >= PRD_VERSION_0_2)
    {
        uint64_t bofTime = prdMeta->bofTime;
        uint64_t eofTime = prdMeta->eofTime;
        if (prdHeader.version >= PRD_VERSION_0_4)
        {
            bofTime |= (uint64_t)prdMeta->bofTimeHigh << 32;
            eofTime |= (uint64_t)prdMeta->eofTimeHigh << 32;
        }
        imageDesc
            << "\nbofTime=" << bofTime << "us"
            << "\neofTime=" << eofTime << "us";
    }
    if (prdHeader.version >= PRD_VERSION_0_3)
    {
        // Cast 8bit values to 16bit, otherwise ostream processes it as char
        imageDesc
            << "\nroiCount=" << prdMeta->roiCount
            << "\ncolorMask=" << (uint16_t)prdHeader.colorMask
            << "\nflags=0x" << std::hex << (uint16_t)prdHeader.flags << std::dec;
    }

    if (pvcamMeta && prdHeader.version >= PRD_VERSION_0_3
            && (prdHeader.flags & PRD_FLAG_HAS_METADATA))
    {
        const rgn_type &irgn = pvcamMeta->impliedRoi;
        // uns8 type is handled as underlying char by stream, cast it to uint16_t
        imageDesc
            << "\nmeta.header.version=" << (uint16_t)pvcamMeta->header->version
            << "\nmeta.header.frameNr=" << pvcamMeta->header->frameNr
            << "\nmeta.header.roiCount=" << pvcamMeta->header->roiCount
            << "\nmeta.header.timeBof=" << pvcamMeta->header->timestampBOF
            << "\nmeta.header.timeEof=" << pvcamMeta->header->timestampEOF
            << "\nmeta.header.timeResNs=" << pvcamMeta->header->timestampResNs
            << "\nmeta.header.expTime=" << pvcamMeta->header->exposureTime
            << "\nmeta.header.expTimeResNs=" << pvcamMeta->header->exposureTimeResNs
            << "\nmeta.header.roiTimeResNs=" << pvcamMeta->header->roiTimestampResNs
            << "\nmeta.header.bitDepth=" << (uint16_t)pvcamMeta->header->bitDepth
            << "\nmeta.header.colorMask=" << (uint16_t)pvcamMeta->header->colorMask
            << "\nmeta.header.flags=" << (uint16_t)pvcamMeta->header->flags
            << "\nmeta.header.extMdSize=" << pvcamMeta->header->extendedMdSize
            << "\nmeta.extMdSize=" << pvcamMeta->extMdDataSize
            << "\nmeta.impliedRoi=[" << irgn.s1 << "," << irgn.s2 << "," << irgn.sbin
                << "," << irgn.p1 << "," << irgn.p2 << "," << irgn.pbin << "]"
            << "\nmeta.roiCapacity=" << pvcamMeta->roiCapacity
            << "\nmeta.roiCount=" << pvcamMeta->roiCount;
        for (int n = 0; n < pvcamMeta->roiCount; ++n)
        {
            const md_frame_roi &roi = pvcamMeta->roiArray[n];
            const auto roiHdr = roi.header;
            if (roiHdr->flags & PL_MD_ROI_FLAG_INVALID)
                continue; // Skip invalid regions
            const rgn_type &rgn = roiHdr->roi;
            imageDesc
                << "\nmeta.roi[" << n << "].header.roiNr=" << roiHdr->roiNr;
            if (pvcamMeta->header->flags & PL_MD_FRAME_FLAG_ROI_TS_SUPPORTED)
            {
                imageDesc
                    << "\nmeta.roi[" << n << "].header.timeBor=" << roiHdr->timestampBOR
                    << "\nmeta.roi[" << n << "].header.timeEor=" << roiHdr->timestampEOR;
            }
            imageDesc
                << "\nmeta.roi[" << n << "].header.roi=[" << rgn.s1 << ","
                    << rgn.s2 << "," << rgn.sbin << "," << rgn.p1 << ","
                    << rgn.p2 << "," << rgn.pbin << "]";
            imageDesc
                << "\nmeta.roi[" << n << "].header.flags=" << (uint16_t)roiHdr->flags
                << "\nmeta.roi[" << n << "].header.extMdSize=" << roiHdr->extendedMdSize
                << "\nmeta.roi[" << n << "].dataSize=" << roi.dataSize
                << "\nmeta.roi[" << n << "].extMdSize=" << roi.extMdDataSize;
        }
    }

    return imageDesc.str();
}
