#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2017 Division of Medical Image Computing, German Cancer Research Center (DKFZ)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# author: kleina
print("here1")
import os
import sys

sys.path.append("../../../")
sys.path.append("../../")
sys.path.append("../")
print("here1")
from configs.Config_unet_boneseg import get_config
from experiments.UNetExperiment import UNetExperiment
print("here2")
c = get_config()

#if c.seg_input == '' or c.seg_output_dir == '':
 #   print('Usage: python segment.py -seg_input <> -seg_output_dir')
 #   exit(0)

c.seg_load_network_path = network_path
print("here3")
#print('Segmenting {}'.format(c.seg_input))
exp = UNetExperiment(config=c, name=c.name, n_epochs=c.n_epochs,
                     seed=42, append_rnd_to_name=c.append_rnd_string, globs=None,
                     # visdomlogger_kwargs={"auto_start": c.start_visdom},
                     # loggers={
                     #     "visdom": ("visdom", {"auto_start": c.start_visdom}),
                     # }
                     )
print("here4")
output_image = exp.segment(sitk_image=sitk_image)