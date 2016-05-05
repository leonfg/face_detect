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

void lineTest(int n)
{
    //get_sram_origin()->control_info.unused = n;
    //e_wait(E_CTIMER_1, 50000);
    return;
}

////////////////////////////////////////////////////////////////////////////////
// Detection
/**
 * Calculate decision based on value of LBP feature
 *
 * @param scan_lines: Pointer to pointers to scan lines of current detection window.
 * @param x: Horizontal coordinate of current detection window.
 * @param node Classifier node used to make decision.
 * @return Decision value: 0 or 1.
 */
static int device_calc_lbp_decision (
    unsigned char const *const *scan_lines,
    int x,
    EpNodeDecision const *const node
) {
    int const feature = node->feature;

    //Shifting position according to LBP feature position
    scan_lines += feature >> 24;
    x += (feature >> 16) & 255;

    int sum00, sum01, sum02,
        sum10, sum11, sum12,
        sum20, sum21, sum22;

    int const feature_width  =  feature       & 255,
              feature_height = (feature >> 8) & 255;

    if( feature_width == 1) {

        if(feature_height == 1) { //Optimization for block 1 by 1 pixel

            unsigned char const *const sl0 = scan_lines[0] + x,
                                *const sl1 = scan_lines[1] + x,
                                *const sl2 = scan_lines[2] + x;

            sum00 = sl0[0]; sum01 = sl0[1]; sum02 = sl0[2];
            sum10 = sl1[0]; sum11 = sl1[1]; sum12 = sl1[2];
            sum20 = sl2[0]; sum21 = sl2[1]; sum22 = sl2[2];

        } else { //2 samples per block (vertically stretched)

            int const step_y = (feature_height - 1) >> 2;

            int const y1 = step_y,
                      y2 = feature_height - step_y - 1,
                      y3 = y1 + feature_height,
                      y4 = y2 + feature_height,
                      y5 = y3 + feature_height,
                      y6 = y4 + feature_height;

            unsigned char const *const sl0 = scan_lines[y1] + x,
                                *const sl1 = scan_lines[y2] + x,
                                *const sl2 = scan_lines[y3] + x,
                                *const sl3 = scan_lines[y4] + x,
                                *const sl4 = scan_lines[y5] + x,
                                *const sl5 = scan_lines[y6] + x;

            sum00 = sl0[0] + sl1[0]; sum01 = sl0[1] + sl1[1]; sum02 = sl0[2] + sl1[2];
            sum10 = sl2[0] + sl3[0]; sum11 = sl2[1] + sl3[1]; sum12 = sl2[2] + sl3[2];
            sum20 = sl4[0] + sl5[0]; sum21 = sl4[1] + sl5[1]; sum22 = sl4[2] + sl5[2];

        }

    } else {

        int const step_x = (feature_width  - 1) >> 2;

        int const x1 = step_x,
                  x2 = feature_width - step_x - 1,
                  x3 = x1 + feature_width,
                  x4 = x2 + feature_width,
                  x5 = x3 + feature_width,
                  x6 = x4 + feature_width;

        if(feature_height == 1) { //2 samples per block (horizontally stretched)

            unsigned char const *const sl0 = scan_lines[0] + x,
                                *const sl1 = scan_lines[1] + x,
                                *const sl2 = scan_lines[2] + x;

            sum00 = sl0[x1] + sl0[x2]; sum01 = sl0[x3] + sl0[x4]; sum02 = sl0[x5] + sl0[x6];
            sum10 = sl1[x1] + sl1[x2]; sum11 = sl1[x3] + sl1[x4]; sum12 = sl1[x5] + sl1[x6];
            sum20 = sl2[x1] + sl2[x2]; sum21 = sl2[x3] + sl2[x4]; sum22 = sl2[x5] + sl2[x6];

        } else { //Large blocks are sampled using 4 samples per block

            int const step_y = (feature_height - 1) >> 2;

            int const y1 = step_y,
                      y2 = feature_height - step_y - 1,
                      y3 = y1 + feature_height,
                      y4 = y2 + feature_height,
                      y5 = y3 + feature_height,
                      y6 = y4 + feature_height;

            unsigned char const *const sl0 = scan_lines[y1] + x,
                                *const sl1 = scan_lines[y2] + x,
                                *const sl2 = scan_lines[y3] + x,
                                *const sl3 = scan_lines[y4] + x,
                                *const sl4 = scan_lines[y5] + x,
                                *const sl5 = scan_lines[y6] + x;

            sum00 = sl0[x1] + sl0[x2] + sl1[x1] + sl1[x2];
            sum01 = sl0[x3] + sl0[x4] + sl1[x3] + sl1[x4];
            sum02 = sl0[x5] + sl0[x6] + sl1[x5] + sl1[x6];

            sum10 = sl2[x1] + sl2[x2] + sl3[x1] + sl3[x2];
            sum11 = sl2[x3] + sl2[x4] + sl3[x3] + sl3[x4];
            sum12 = sl2[x5] + sl2[x6] + sl3[x5] + sl3[x6];

            sum20 = sl4[x1] + sl4[x2] + sl5[x1] + sl5[x2];
            sum21 = sl4[x3] + sl4[x4] + sl5[x3] + sl5[x4];
            sum22 = sl4[x5] + sl4[x6] + sl5[x5] + sl5[x6];

        }

    }

    //Two's complement arithmetic required!

    unsigned int const sign = 1 << 31;

    int const subset_index =
        ( ( ( (unsigned int)~(sum00 - sum11) ) & sign ) >> 29 ) |
        ( ( ( (unsigned int)~(sum01 - sum11) ) & sign ) >> 30 ) |
        (   ( (unsigned int)~(sum02 - sum11) )          >> 31 ) ;

    int const bit_index =
        ( ( ( (unsigned int)~(sum12 - sum11) ) & sign ) >> 27 ) |
        ( ( ( (unsigned int)~(sum22 - sum11) ) & sign ) >> 28 ) |
        ( ( ( (unsigned int)~(sum21 - sum11) ) & sign ) >> 29 ) |
        ( ( ( (unsigned int)~(sum20 - sum11) ) & sign ) >> 30 ) |
        (   ( (unsigned int)~(sum10 - sum11) )          >> 31 ) ;

    return (node->subsets[subset_index] >> bit_index) & 1;
}

/**
 * Classify single image position as object or not_object.
 * This function works as virtual machine interpreting instructions stored
 * in list pointed by "node" variable. It can understand 3 instructions:
 * NODE_DECISION: calculate value of specified feature and modify object_score
 *   by specified value if feature value is in specified subset.
 * NODE_STAGE: compare object_score accumulated so far with specified threshold.
 *   if value is less than threshold then return 0 otherwise continue.
 * NODE_FINAL: return 1.
 * For performance reasons it is supposed that first node is always
 * NODE_DECISION, and two NODE_STAGE nodes are never go in succession.
 * @param image_data Position in memory where to sample data from.
 * @return 1 for positive classification. Zero otherwise.
 */
static int classify (
    unsigned char const *const *const scan_lines,
    int const x
) {
    //Skipping initial META node
	char const *node = (char *)((EpCoreBank3 *)BANK3)->buf_classifier + sizeof(EpNodeMeta);

    //Node after META is always NODE_DECISION
    int object_score = ((EpNodeDecision const *)node)->score &
        -device_calc_lbp_decision(scan_lines, x, (EpNodeDecision const *)node);
    node += sizeof(EpNodeDecision);

    while(1) {
        if(!*node) { //NODE_DECISION
            object_score += ((EpNodeDecision const *)node)->score &
                -device_calc_lbp_decision(scan_lines, x, (EpNodeDecision const *)node);
            node += sizeof(EpNodeDecision);
        } else { //NODE_STAGE
            if(object_score < ((EpNodeStage *)node)->threshold)
                return 0;
            node += sizeof(EpNodeStage);

            if(*node)
                return 1; //NODE_FINAL

            //NODE_DECISION is after NODE_STAGE if no NODE_FINAL found
            object_score = ((EpNodeDecision const *)node)->score &
                -device_calc_lbp_decision(scan_lines, x, (EpNodeDecision const *)node);
            node += sizeof(EpNodeDecision);
        }
    }

    return 0; //This point is unreachable
}

void device_detect_single_scale(void) {
	char const *const classifier_data = (char const *)((EpCoreBank3 *)BANK3)->buf_classifier;

    //assert (((EpNodeMeta const *)classifier_data)->id == NODE_META);

    int const window_width  = ((EpNodeMeta const *)classifier_data)->window_width;
    int const window_height = ((EpNodeMeta const *)classifier_data)->window_height;

	int const process_width = ((EpCoreBank1 *)BANK1)->task_item.width + 1 - window_width;
	int const process_height = ((EpCoreBank1 *)BANK1)->task_item.height + 1 - window_height;

	int const image_step = ((EpCoreBank1 *)BANK1)->task_item.step;
	int const scan_mode = ((EpCoreBank1 *)BANK1)->task_item.scan_mode;

    //To do without multiplications we use this small array of pointers
    unsigned char const *scan_lines[window_height];
	scan_lines[0] = (unsigned char const *)((EpCoreBank1 *)BANK1)->buf_tile;
    for(int y = 1; y < window_height; ++y)
        scan_lines[y] = scan_lines[y - 1] + image_step;

    int num_objects = 0;
#if 1
    for(int y = 0; y < process_height; ++y) {
	//e_wait(E_CTIMER_1, 5000);
        int const x_start = scan_mode == SCAN_FULL ? 0 : (y + scan_mode) & 1;
        int const x_step = scan_mode == SCAN_FULL ? 1 : 2;

        for(int x = x_start; x < process_width; x += x_step) {
	//e_wait(E_CTIMER_1, 5000);
            if( !classify(scan_lines, x) ) continue;

			((EpCoreBank1 *)BANK1)->task_item.objects[num_objects] = x | (y << 16);
            ++num_objects;
            if(num_objects == MAX_DETECTIONS_PER_TILE)
                break;
        }

        if(num_objects == MAX_DETECTIONS_PER_TILE)
            break;

        for(int yt = 0; yt < window_height; ++yt)
	{
	    //e_wait(E_CTIMER_1, 5000);
            scan_lines[yt] += image_step;
	}
    }
#endif
	((EpCoreBank1 *)BANK1)->task_item.items_count = num_objects;
}

/**
 * Clone image tile to core memory
 * @param src_buf pointer to the source image in shared memory
 * @param src_step step of source image
 */
static void subimage_clone_to_core (
    unsigned char volatile const *const src_buf,
    int const src_step
) {
	//lineTest(17);
	EpTaskItem const *const task_item = &((EpCoreBank1 *)BANK1)->task_item;

	//lineTest(2);
    int const result_step   = task_item->step,
              result_height = task_item->height,
              result_area   = task_item->area;

	//lineTest(3);
    unsigned char volatile const *source_pixels = src_buf + task_item->offset;
	unsigned char *result_pixels = ((EpCoreBank1 *)BANK1)->buf_tile;

	//lineTest(4);

    if(src_step == result_step) {
        dma_transfer(result_pixels, source_pixels, result_area, 1);
	lineTest(5);
    }
#if 1
    else {
        for(int line = 0; line < result_height; ++line) {
            lineTest(line+2000);
            dma_transfer(result_pixels, source_pixels, result_step, 1/*line == result_height - 1*/);
            result_pixels += result_step;
            source_pixels += src_step;
	    //e_wait(E_CTIMER_1, 5000);
	    lineTest(line+1000);
        }
    }
#endif
}

/**
 * @return pointer to next task to take
 */
static EpTaskItem volatile *get_next_task() {

    int const task_cur = atomic_increment(&get_sram_origin()->control_info.task_to_take, get_sram_origin()->control_info.task_count);

    if(task_cur < get_sram_origin()->control_info.task_count)
        return get_sram_origin()->tasks + task_cur;

    return 0;
}

/**
 * Load classifier in local cores bank from shared memory
 */
static void load_classifier() {
	dma_transfer(((EpCoreBank3 *)BANK3)->buf_classifier, get_sram_origin()->buf_classifier, MAX_CLASSIFIER_BYTES, 0);
}

/**
 * Process task list on core
 */
void device_process_tasks(void) {
	lineTest(1);
    load_classifier();
	lineTest(11);
	((EpCoreBank1 *)BANK1)->timer.value = 0;

    while(1) {
//e_wait(E_CTIMER_1, 5000);
	lineTest(12);
        EpTaskItem volatile *const cur_task = get_next_task();
        if(cur_task == 0)
            break;
	lineTest(13);
		dma_transfer(&((EpCoreBank1 *)BANK1)->task_item, cur_task, sizeof(EpTaskItem), 1);
	lineTest(14);
		EpImageProp  volatile const *const cur_img_prop = get_sram_origin()->imgs_prop + ((EpCoreBank1 *)BANK1)->task_item.image_index;
	lineTest(15);
        unsigned char volatile const *const cur_img_buf = get_sram_origin()->imgs_buf + cur_img_prop->data_offset;
	lineTest(16);
        subimage_clone_to_core(cur_img_buf, cur_img_prop->step);

	lineTest(7);

        unsigned int const start_ticks = start_timer();
	
	lineTest(8);

        device_detect_single_scale();
	
	lineTest(9);
#if 1
        if(TIMER_VALUE_SHIFT)
			((EpCoreBank1 *)BANK1)->timer.value += (start_ticks - stop_timer() + (1 << (TIMER_VALUE_SHIFT - 1))) >> TIMER_VALUE_SHIFT;
        else
			((EpCoreBank1 *)BANK1)->timer.value += start_ticks - stop_timer();
#endif
		if (((EpCoreBank1 *)BANK1)->task_item.items_count > 0) //Sending results back
			dma_transfer(cur_task, &((EpCoreBank1 *)BANK1)->task_item, sizeof(EpTaskItem), 0);

        atomic_increment(&get_sram_origin()->control_info.task_finished, get_sram_origin()->control_info.task_count);
    }
	lineTest(20);
    //Sending timer to shared memory
    //ToDo: it can happen that host will read these timers before they will be written completely

    int const timer_cur = atomic_increment(&get_sram_origin()->control_info.timer_index, 4096);
	dma_transfer(get_sram_origin()->timers + timer_cur, &((EpCoreBank1 *)BANK1)->timer, sizeof(EpTimerBuf), 1);
	lineTest(21);
}
