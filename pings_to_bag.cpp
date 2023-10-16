#include <rosbag/bag.h>
#include "raw_ping.h"
#include "ping.h"
#include <iostream>
#include <marine_acoustic_msgs/RawSonarImage.h>
#include <tf2_msgs/TFMessage.h>

int main(int argc, char *argv[])
{

  rosbag::Bag bag;

  geometry_msgs::TransformStamped transform;
  transform.child_frame_id = "usbl";
  transform.header.frame_id = "project11/drix_8/base_link";
  transform.transform.rotation.y = 1.0; // 180 rotation around y axis

  for(int i = 1; i < argc; i++)
  {
    std::cout << argv[i] << std::endl;
    if (i == 1)
      bag.open(std::string(argv[i])+".bag", rosbag::bagmode::Write);
    RawPing rp(argv[i]);
    Ping ping(rp);

    marine_acoustic_msgs::RawSonarImage image;
    
    int32_t sec = std::chrono::duration_cast<std::chrono::seconds>(ping.timestamp().time_since_epoch()).count();
    int32_t nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(ping.timestamp().time_since_epoch()).count() % 1000000000UL;
    image.header.stamp.sec = sec;
    image.header.stamp.nsec = nsec;
    image.header.frame_id = "usbl";

    image.sample_rate = ping.sampleRate();
    image.tx_delays.push_back(0.0);
    image.tx_angles.push_back(0.0);
    image.rx_angles.push_back(0.0);

    image.samples_per_beam = ping.samples().size();

    image.ping_info.sound_speed = 1500.0;

    image.image.beam_count = 1;
    image.image.dtype = marine_acoustic_msgs::SonarImageData::DTYPE_FLOAT32;

    image.image.data.resize(ping.samples().size()*4);
    auto data_as_floats = reinterpret_cast<float*>(image.image.data.data());
    for(int i = 0; i < ping.samples().size(); i++)
      data_as_floats[i] = ping.samples()[i];
    tf2_msgs::TFMessage tf_message;
    transform.header.stamp = image.header.stamp;
    tf_message.transforms.push_back(transform);
    bag.write("/tf", transform.header.stamp, tf_message);
    bag.write("usbl/pings", image.header.stamp, image);
  }
  bag.close();
  return 0;
}
