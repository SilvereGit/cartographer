/*
 * Copyright 2016 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cartographer/io/ply_custom_writing_points_processor.h"

#include <iomanip>
#include <sstream>
#include <string>

#include "absl/memory/memory.h"
#include "cartographer/common/lua_parameter_dictionary.h"
#include "cartographer/io/points_batch.h"
#include "glog/logging.h"

namespace cartographer {
namespace io {

namespace {

// Writes the PLY header claiming 'num_points' will follow it into
// 'output_file'.
//void WriteCustomBinaryPlyHeader(const bool has_color, const bool has_intensity, const int64 num_points, FileWriter* const file_writer) {
void WriteCustomBinaryPlyHeader(const bool has_color, const bool has_intensity, const bool has_ring, const int64 num_points, FileWriter* const file_writer) {
  std::string color_header = !has_color ? ""
                                        : "property uchar red\n"
                                          "property uchar green\n"
                                          "property uchar blue\n";
  std::string intensity_header = !has_intensity ? ""
                                        : "property float intensity\n";
  std::string ring_header = !has_ring ? ""
                                        : "property ushort ring\n";

  std::ostringstream stream;
  stream << "ply\n"
         << "format binary_little_endian 1.0\n"
         << "comment Point cloud hypnotized and smoothly lured from the basket by the almighty snake charmer: Kamikaze Viper\n"
         << "element vertex " << std::setw(15) << std::setfill('0')
         << num_points << "\n"
         << "property float x\n"
         << "property float y\n"
         << "property float z\n"
         << color_header
         << intensity_header
         << "property double time\n"
         << ring_header << "end_header\n";
  const std::string out = stream.str();
  CHECK(file_writer->WriteHeader(out.data(), out.size()));
}

void WriteCustomBinaryPlyPointCoordinate(const Eigen::Vector3f& point,
                                   FileWriter* const file_writer) {
  char buffer[12];
  memcpy(buffer, &point[0], sizeof(float));
  memcpy(buffer + 4, &point[1], sizeof(float));
  memcpy(buffer + 8, &point[2], sizeof(float));
  CHECK(file_writer->Write(buffer, 12));
}

void WriteCustomBinaryPlyPointColor(const FloatColor& color, FileWriter* const file_writer) {
  
  uint8_t r = color[0], g = color[1], b = color[2];
  //uint32_t rgb = ((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b);
  //float c = *reinterpret_cast<float*>(&rgb);
  
  char buffer[1];
  memcpy(buffer, &r, sizeof(uint8_t));
  CHECK(file_writer->Write(buffer, sizeof(uint8_t)));
  memcpy(buffer, &g, sizeof(uint8_t));
  CHECK(file_writer->Write(buffer, sizeof(uint8_t)));
  memcpy(buffer, &b, sizeof(uint8_t));
  CHECK(file_writer->Write(buffer, sizeof(uint8_t)));
  //CHECK(file_writer->Write(reinterpret_cast<const char*>(color.data()), color.size()));
}

void WriteCustomBinaryPlyPointTime(const double& time, FileWriter* const file_writer) {
  //std::cout << std::setprecision(17) << "time " << time << " " << sizeof(double) << std::endl;
  //double tim = time;// - (double)1513507200.0;
  char buffer[8];
  memcpy(buffer, &time, sizeof(double));
  CHECK(file_writer->Write(buffer, 8));
  //double time2;
  //memcpy(&time2, buffer, sizeof(double));
  //std::cout << std::setprecision(17) << "time2 " << time2 << " " << sizeof(double) << std::endl;
}

void WriteCustomBinaryPlyPointIntensity(const float& intensity, FileWriter* const file_writer) {

  char buffer[4];
  memcpy(buffer, &intensity, sizeof(float));
  CHECK(file_writer->Write(buffer, 4));
}

void WriteCustomBinaryPlyPointRing(const uint16_t& ring, FileWriter* const file_writer) {

  char buffer[sizeof(uint16_t)];
  memcpy(buffer, &ring, sizeof(uint16_t));
  CHECK(file_writer->Write(buffer, sizeof(uint16_t)));
}

}  // namespace

std::unique_ptr<PlyCustomWritingPointsProcessor>
PlyCustomWritingPointsProcessor::FromDictionary(
    const FileWriterFactory& file_writer_factory,
    common::LuaParameterDictionary* const dictionary,
    PointsProcessor* const next) {
  return absl::make_unique<PlyCustomWritingPointsProcessor>(
      file_writer_factory(dictionary->GetString("filename")), next);
}

PlyCustomWritingPointsProcessor::PlyCustomWritingPointsProcessor(
    std::unique_ptr<FileWriter> file_writer, PointsProcessor* const next)
    : next_(next),
      num_points_(0),
      has_colors_(false),
      has_intensity_(false),
      has_rings_(false),
      file_(std::move(file_writer)) {}

/*
PointsProcessor::FlushResult PlyCustomWritingPointsProcessor::Flush() {
  WriteCustomBinaryPlyHeader(has_colors_, has_intensity_, num_points_, file_.get());
  CHECK(file_->Close()) << "Closing PLY file_writer failed.";

  switch (next_->Flush()) {
    case FlushResult::kFinished:
      return FlushResult::kFinished;

    case FlushResult::kRestartStream:
      LOG(FATAL) << "PLY generation must be configured to occur after any "
                    "stages that require multiple passes.";
  }
  LOG(FATAL);
}
*/

PointsProcessor::FlushResult PlyCustomWritingPointsProcessor::Flush() {
  WriteCustomBinaryPlyHeader(has_colors_, has_intensity_, has_rings_, num_points_, file_.get());
  CHECK(file_->Close()) << "Closing PLY file_writer failed.";

  switch (next_->Flush()) {
    case FlushResult::kFinished:
      return FlushResult::kFinished;

    case FlushResult::kRestartStream:
      LOG(FATAL) << "PLY generation must be configured to occur after any "
                    "stages that require multiple passes.";
  }
  LOG(FATAL);
}

void PlyCustomWritingPointsProcessor::Process(std::unique_ptr<PointsBatch> batch) {
  if (batch->points.empty()) {
    next_->Process(std::move(batch));
    return;
  }

  if (num_points_ == 0) {
    has_colors_ = !batch->colors.empty();
    has_intensity_ = !batch->intensities.empty();
    has_rings_ = !batch->rings.empty();
    //WriteCustomBinaryPlyHeader(has_colors_, has_intensity_, 0, file_.get());
    WriteCustomBinaryPlyHeader(has_colors_, has_intensity_, has_rings_, 0, file_.get());
  }
  if (has_colors_) {
    CHECK_EQ(batch->points.size(), batch->colors.size())
        << "First PointsBatch had colors, but encountered one without. "
           "frame_id: "
        << batch->frame_id;
  }
  if (has_intensity_) {
    CHECK_EQ(batch->points.size(), batch->intensities.size())
        << "First PointsBatch had intesities, but encountered one without. "
           "frame_id: "
        << batch->frame_id;
  }
  if (has_rings_) {
    CHECK_EQ(batch->points.size(), batch->rings.size())
        << "First PointsBatch had rings, but encountered one without. "
           "frame_id: "
        << batch->frame_id;
  }
  for (size_t i = 0; i < batch->points.size(); ++i) {
    //WriteCustomBinaryPlyPointTime(ToUniversalDouble(batch->start_time), file_.get());
    WriteCustomBinaryPlyPointCoordinate(batch->points[i].position, file_.get());
    if (has_colors_) {
	WriteCustomBinaryPlyPointColor(batch->colors[i], file_.get());
    }
    if (has_intensity_) {
      WriteCustomBinaryPlyPointIntensity(batch->intensities[i], file_.get());
    }
    //std::cout << std::setprecision(17) << "start_time_unix " << batch->start_time_unix << std::endl;
    WriteCustomBinaryPlyPointTime(batch->start_time_unix, file_.get());
    if (has_rings_) {
      WriteCustomBinaryPlyPointRing(batch->rings[i], file_.get());
    }
    ++num_points_;
  }
  next_->Process(std::move(batch));
}

}  // namespace io
}  // namespace cartographer
