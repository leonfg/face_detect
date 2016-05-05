/* <title of the code in this file>
   Copyright (C) 2012 Adapteva, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program, see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>. */

#include <omp.h>

#include "ep_cascade_detector.hpp"

#ifdef __OPENCV_OBJDETECT_HPP__
    #include "cascadedetect.hpp"
#endif

namespace ep
{
    /**
     * Index of the rectangle and amount of its intersection with current rectangle
     */
    struct RectsIntersection {
        int index;
        float amount;
        inline RectsIntersection(int const index, float const amount):
            index(index), amount(amount)
        { ; }
    };

    /**
     * List of intersections with current rectangle
     */
    struct IntersectionsList {
        float total_amount;
        std::vector<RectsIntersection> intersections;
        inline IntersectionsList(void):
            total_amount(1.0f), intersections() //Item intersects with itself, hence 1.0f here
        { ; }
    };

    /**
     * Remove rectangle from list of lists of intersection
     */
    void remove_item(std::vector<IntersectionsList> &intersections, int const remove_index) {
        IntersectionsList &item( intersections[remove_index] );

        for(int i(0); i < static_cast<int>( item.intersections.size() ); ++i) {
            IntersectionsList &opposite_list( intersections[item.intersections[i].index] );

            for(int j(0); j < static_cast<int>( opposite_list.intersections.size() ); ++j) {
                if(opposite_list.intersections[j].index != remove_index)
                    continue;

                opposite_list.total_amount -= opposite_list.intersections[j].amount;
                opposite_list.intersections[j] = opposite_list.intersections.back();
                opposite_list.intersections.pop_back();

                break;
            }
        }

        item.total_amount = 0.0f; //Zero value means that rectangle is removed
        item.intersections.clear();
    }

    /**
     * Length of intersection of two segments [a1, a2] and [b1, b2]
     */
    inline float intersection_len(float const a1, float const a2, float const b1, float const b2) {
        return std::max(std::min(a2, b2) - std::max(a1, b1), 0.0f);
    }

    /**
     * Amount of intersection of two rectangles
     * @return value from 0.0f to 1.0f which is equal to ratio between rectangles intersection area and rectangles union area
     */
    inline float intersection_amount(EpRect const &r1, EpRect const &r2) {
        float amount( intersection_len(r1.x, r1.x + r1.width, r2.x, r2.x + r2.width) );
        if(!amount) return 0.0f;
        amount *= intersection_len(r1.y, r1.y + r1.height, r2.y, r2.y + r2.height);
        if(!amount) return 0.0f;
        return amount / (r1.width * r1.height + r2.width * r2.height - amount);
    }

    /**
     * Round given floating points coordinates to rectangle with integer coordinates
     */
    inline cv::Rect round_rect(float const x, float const y, float const width, float const height) {
        int const xi1( cvRound(x        ) ), yi1( cvRound(y         ) ),
                  xi2( cvRound(x + width) ), yi2( cvRound(y + height) );
        return cv::Rect(xi1, yi1, xi2 - xi1, yi2 - yi1);
    }

    /**
     * Group rectangles
     * @param ep_rectangles: source detections
     * @param rectangles: resulting grouped detections
     * @param min_neighbors: if zero then source rectangles will be just copied to result. Otherwise grouping is performed. Groups containing less than min_neighbors are discarded
     */
    void group_rectangles (
        EpRectList const &ep_rectangles,
        std::vector<cv::Rect> &rectangles,
        int const min_neighbors
    ) {
        rectangles.clear();

        if(min_neighbors < 1) {
            for(int i(0); i < ep_rectangles.count; ++i)
                rectangles.push_back( round_rect (
                    ep_rectangles.data[i].x    , ep_rectangles.data[i].y     ,
                    ep_rectangles.data[i].width, ep_rectangles.data[i].height
                ) );
            return;
        }

        std::vector<IntersectionsList> intersections(ep_rectangles.count);

        for(int i1 = 0; i1 < ep_rectangles.count - 1; ++i1) {
            for(int i2(i1 + 1); i2 < ep_rectangles.count; ++i2) {
                float const amount( intersection_amount( ep_rectangles.data[i1], ep_rectangles.data[i2] ) );
                if(amount < 0.5f)
                    continue;

                intersections[i1].total_amount += amount;
                intersections[i1].intersections.push_back( RectsIntersection(i2, amount) );

                intersections[i2].total_amount += amount;
                intersections[i2].intersections.push_back( RectsIntersection(i1, amount) );
            }
        }

        while(true) {
            float best_amount(0.0f);
            int best_index(-1);

            for(int i(0); i < ep_rectangles.count; ++i) {
                if(static_cast<int>( intersections[i].intersections.size() ) + 1 < min_neighbors)
                    continue;

                if(intersections[i].total_amount > best_amount) {
                    best_amount = intersections[i].total_amount;
                    best_index = i;
                }
            }

            if(best_index < 0)
                break;

            IntersectionsList &best_list( intersections[best_index] );

            EpRect &rectangle( ep_rectangles.data[best_index] );

            float sum_x(rectangle.x), sum_y(rectangle.y),
                  sum_width(rectangle.width), sum_height(rectangle.height);

            float const sum_k(best_list.total_amount);

            while( best_list.intersections.size() ) {
                float const k(best_list.intersections.front().amount);
                int const index(best_list.intersections.front().index);

                EpRect const &rectangle( ep_rectangles.data[index] );
                sum_x += rectangle.x * k; sum_y += rectangle.y * k;
                sum_width += rectangle.width * k; sum_height += rectangle.height * k;

                remove_item(intersections, index);
            }

            best_list.total_amount = 0.0f;
            best_list.intersections.clear();

            rectangles.push_back( round_rect (
                sum_x     / sum_k, sum_y      / sum_k,
                sum_width / sum_k, sum_height / sum_k
            ) );
        }
    }

    /**
     * Wrapper around corresponding C routine (@see ep_detect_multi_scale).
     * In addition this routine makes objects grouping.
     * @param min_neighbors: minimal number of detections in detection group.
     *                       if this value is zero then grouping is disabled.
     */
    EpErrorCode detect_multi_scale (
        cv::Mat               const &image,
        CascadeClassifier     const &classifier,
        std::vector<cv::Rect>       &objects,
        int                   const  min_neighbors,
        EpScanMode            const  scan_mode,
        EpDetectionMode       const  detection_mode,
        int                          num_cores,
        std::string           const &log_file
    ) {
        EpImage ep_image_orig = { image.data, image.cols, image.rows, static_cast<int>(image.step) };
        //ToDo: ideally aligned copy should be created directly in shared memory
        EpImage ep_image_aligned = ep_image_clone(&ep_image_orig);

        EpRectList ep_objects( ep_rect_list_create_empty() );

        EpErrorCode result(ERR_ARGUMENT);

        if(detection_mode == DET_HOST)
            result = ep_detect_multi_scale_host(&ep_image_aligned, classifier.get_data(), &ep_objects, scan_mode);

        if(detection_mode == DET_DEVICE)
            result = ep_detect_multi_scale_device (
                &ep_image_aligned,
                 classifier.get_data(),
                &ep_objects,
                 scan_mode,
                 num_cores,
                log_file.length() ? log_file.c_str() : NULL
            );

        group_rectangles(ep_objects, objects, min_neighbors);

        ep_rect_list_release(&ep_objects);

        ep_image_release(&ep_image_aligned);

        return result;
    }

#ifdef __OPENCV_OBJDETECT_HPP__
    class ClassifierAccessor: public cv::CascadeClassifier {
        friend EpCascadeClassifier convert_cascade(cv::CascadeClassifier const &cv_classifier);
    };

    class FeatureAccessor: public cv::LBPEvaluator {
        friend EpCascadeClassifier convert_cascade(cv::CascadeClassifier const &cv_classifier);
    };

    /**
     * Count number of unit bits in given value
     */
    int count_bits(unsigned int x) {
        x = (x & 0x55555555) + ( (x >>  1) & 0x55555555 );
        x = (x & 0x33333333) + ( (x >>  2) & 0x33333333 );
        x = (x & 0x0F0F0F0F) + ( (x >>  4) & 0x0F0F0F0F );
        x = (x & 0x00FF00FF) + ( (x >>  8) & 0x00FF00FF );
        x = (x & 0x0000FFFF) + ( (x >> 16) & 0x0000FFFF );
        return x;
    }

    /**
     * Convert cv::CascadeClassifier into EpCascadeClassifier usable by ep_detect_multi_scale
     */
    EpCascadeClassifier convert_cascade(cv::CascadeClassifier const &cv_classifier) {

        if( cv_classifier.empty() )
            return ep_classifier_create_empty();

        //Formally the line below is undefined behavior, but there is no better way to access cv::CascadeClassifier data without modifying OpenCV code or duplicating all the data once more
        ClassifierAccessor const &classifier_accessor( reinterpret_cast<ClassifierAccessor const &>(cv_classifier) );
        ClassifierAccessor::Data const &data(classifier_accessor.data);
        if( !data.isStumpBased || data.stageType != 0 || data.featureType != cv::FeatureEvaluator::LBP )
            return ep_classifier_create_empty(); //Unknown type of classifier
        //Dynamic checking of actual derived class type for features
        if( !dynamic_cast<cv::LBPEvaluator const *>(classifier_accessor.featureEvaluator.obj) )
            return ep_classifier_create_empty();
        //Another hack
        FeatureAccessor const &feature_accessor( *reinterpret_cast<FeatureAccessor const *>(classifier_accessor.featureEvaluator.obj) );

        int classifier_size( sizeof(EpNodeMeta) ); //One meta node will be added
        for(int stage_index(0); stage_index < static_cast<int>( data.stages.size() ); ++stage_index) {
            classifier_size += sizeof(EpNodeDecision) * data.stages[stage_index].ntrees;
            classifier_size += sizeof(EpNodeStage); //One additional node for stage termination
        }
        classifier_size += sizeof(EpNodeFinal); //One final node

        EpCascadeClassifier const result = {static_cast<char *>( malloc(classifier_size) ), classifier_size};

        {
            EpNodeMeta &node_meta( *reinterpret_cast<EpNodeMeta *>(result.data) );
            node_meta.id = NODE_META;
            node_meta.window_width = data.origWinSize.width;
            node_meta.window_height = data.origWinSize.height;
        }

        char *cur_node( result.data + sizeof(EpNodeMeta) );

        for(int stage_index(0); stage_index < static_cast<int>( data.stages.size() ); ++stage_index) {

            int const first_node(data.stages[stage_index].first);

            int node_threshold( cvRound(data.stages[stage_index].threshold * 65535) );

            for(int node_index(0); node_index < data.stages[stage_index].ntrees; ++node_index) {
                int const global_node_index(first_node + node_index);
                int const feature_id = data.nodes[global_node_index].featureIdx;
                cv::Rect const &block_rect( (*feature_accessor.features)[feature_id].rect );

                int const score_negative( cvRound(data.leaves[global_node_index * 2    ] * 65535) ),
                          score_positive( cvRound(data.leaves[global_node_index * 2 + 1] * 65535) );

                EpNodeDecision &node_decision( *reinterpret_cast<EpNodeDecision *>(cur_node) );

                node_decision.id = NODE_DECISION;
                node_decision.feature = block_rect.width | (block_rect.height << 8) |
                                    (block_rect.x << 16) | (block_rect.y << 24);

                int ones(0); //Number of ones in subset
                for(int subset_index(0); subset_index < 8; ++subset_index) {
                    int const subset( data.subsets[global_node_index * 8 + subset_index] );
                    ones += count_bits(subset);
                    node_decision.subsets[subset_index] = subset;
                }

                //Minimizing probability that value "1" will be hit in the subset
                if(ones <= 128) {
                    node_decision.score = score_negative - score_positive;
                    node_threshold -= score_positive;
                } else {
                    node_decision.score = score_positive - score_negative;
                    node_threshold -= score_negative;
                    for(int subset_index(0); subset_index < 8; ++subset_index)
                        node_decision.subsets[subset_index] = ~node_decision.subsets[subset_index];
                }

                cur_node += sizeof(EpNodeDecision);
            }

            EpNodeStage &node_stage( *reinterpret_cast<EpNodeStage *>(cur_node) );
            node_stage.id = NODE_STAGE;
            node_stage.threshold = node_threshold;
            
            cur_node += sizeof(EpNodeStage);

        }

        EpNodeFinal &node_final( *reinterpret_cast<EpNodeFinal *>(cur_node) );
        node_final.id = NODE_FINAL;

        return result;
    }
#endif

    ////////////////////////////////////////////////////////

    CascadeClassifier::CascadeClassifier(void):
    ep_cascade_classifier( ep_classifier_create_empty() )
    { ; }

    CascadeClassifier::CascadeClassifier(std::string const &file_name):
        ep_cascade_classifier( ep_classifier_load(file_name.c_str(), NULL) )
    { ; }

#ifdef __OPENCV_OBJDETECT_HPP__
    CascadeClassifier::CascadeClassifier(cv::CascadeClassifier const &cv_classifier):
        ep_cascade_classifier( convert_cascade(cv_classifier) )
    { ; }

    CascadeClassifier &CascadeClassifier::operator=(cv::CascadeClassifier const &cv_classifier)
    {
        ep_classifier_release(&ep_cascade_classifier);
        ep_cascade_classifier = convert_cascade(cv_classifier);
        return *this;
    }
#endif

    CascadeClassifier::CascadeClassifier(CascadeClassifier const &classifier):
        ep_cascade_classifier( ep_classifier_clone(&classifier.ep_cascade_classifier) )
    { ; }

    CascadeClassifier::~CascadeClassifier(void) {
        release();
    }

    CascadeClassifier &CascadeClassifier::operator=(CascadeClassifier const &classifier) {
        if(classifier.ep_cascade_classifier.data == ep_cascade_classifier.data)
            return *this;

        ep_classifier_release(&ep_cascade_classifier);
        ep_cascade_classifier = ep_classifier_clone(&classifier.ep_cascade_classifier);

        return *this;
    }

    bool CascadeClassifier::empty(void) const {
        return ep_classifier_is_empty(&ep_cascade_classifier) != 0;
    }

    void CascadeClassifier::release(void) {
        ep_classifier_release(&ep_cascade_classifier);
    }

    EpErrorCode CascadeClassifier::load(std::string const &file_name) {
        release();
        EpErrorCode result;
        ep_cascade_classifier = ep_classifier_load(file_name.c_str(), &result);
        return result;
    }

    EpErrorCode CascadeClassifier::save(std::string const &file_name) const {
        return ep_classifier_save( &ep_cascade_classifier, file_name.c_str() );
    }

    EpCascadeClassifier const *CascadeClassifier::get_data(void) const {
        return &ep_cascade_classifier;
    }

    int CascadeClassifier::get_size(void) const {
        return ep_cascade_classifier.size;
    }
}
