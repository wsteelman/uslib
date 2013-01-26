
#include "Image.hh"

namespace uslib
{

Frame::Frame(uint32 id, uint32 num_channels, uint32 image_map_id,
             uint32 vectors, uint32 samples,
             uint32 max_display_size) :
   m_id(id),
   m_num_channels(num_channels),
   m_map_id(image_map_id),
   m_vectors(vectors),
   m_samples(samples),
   m_width(0),
   m_height(0),
   m_max_display_size(max_display_size),
   m_display(NULL),
   m_focused(NULL)
{
   m_channels = new uint8*[num_channels];
   m_focused = new float[m_samples*m_vectors];
   m_display = new uint8[m_max_display_size];
}

Frame::~Frame()
{
   delete [] m_channels;
   delete [] m_display;
}

err
Frame::AddChannelData(uint32 chnl, uint8 *data)
{
   if (chnl < m_num_channels)
   {
      m_channels[chnl] = data;
      return SUCCESS; 
   }
   return OUTOFRANGE;
}

} // namespace uslib
