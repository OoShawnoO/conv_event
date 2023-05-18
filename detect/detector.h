#ifndef DETECT_DETECTOR_H
#define DETECT_DETECTOR_H
#include <unordered_map>
#include <torch/script.h>               /* torch::jit */
#include <opencv2/opencv.hpp>           /* opencv::cv */
#include "core/include/json/json.h"     /* hzd::json */

namespace hzd {

    class detector {
    private:
        torch::jit::Module module{};
        json items{};
    public:
        explicit detector(const std::string& model_dir_path)
        {
            try{
                module = torch::jit::load(model_dir_path + "/model.pth");
                module.to(torch::kCPU);
                items.load_by_file_name(model_dir_path + "/tags.json");
            }
            catch(...)
            {
                std::cerr << "module load error" << std::endl;
            }
        }

        static torch::Tensor load_img(const std::string& img_path)
        {
            auto img = cv::imread(img_path);
            cv::cvtColor(img,img,cv::COLOR_BGR2RGB);
            cv::Mat transformed_img;
            cv::resize(img,transformed_img,cv::Size(224,224));
            torch::Tensor tensor_img = torch::from_blob(transformed_img.data,{
                transformed_img.rows,transformed_img.cols,3
                },torch::kByte);
            tensor_img = tensor_img.permute({2,0,1});
            tensor_img = tensor_img.toType(torch::kFloat);
            tensor_img = tensor_img.div(255);
            tensor_img = tensor_img.unsqueeze(0);
            tensor_img = tensor_img.to(torch::kCPU);
            return tensor_img;
        }

        static torch::Tensor load_img_bytes(const std::string& img_bytes)
        {
            std::vector<unsigned char> data(img_bytes.begin(),img_bytes.end());
            auto img = cv::imdecode(cv::Mat(data),cv::IMREAD_COLOR);
            cv::cvtColor(img,img,cv::COLOR_BGR2RGB);
            cv::Mat transformed_img;
            cv::resize(img,transformed_img,cv::Size(224,224));
            torch::Tensor tensor_img = torch::from_blob(transformed_img.data,{
                    transformed_img.rows,transformed_img.cols,3
            },torch::kByte);
            tensor_img = tensor_img.permute({2,0,1});
            tensor_img = tensor_img.toType(torch::kFloat);
            tensor_img = tensor_img.div(255);
            tensor_img = tensor_img.unsqueeze(0);
            tensor_img = tensor_img.to(torch::kCPU);
            return tensor_img;
        }

        std::pair<std::string,float> predict(const std::string& img_path)
        {
            torch::Tensor tensor_img = load_img(img_path);
            torch::Tensor output = module.forward({tensor_img}).toTensor();
            auto percent = output.softmax(1);
            int index = output.argmax(1).item<int>();
            return {items[std::to_string(index)],percent[0][index].item<float>()*100};
        };

        std::pair<std::string,float> predict_bytes(const std::string& img_data)
        {
            torch::Tensor tensor_img = load_img_bytes(img_data);
            torch::Tensor output = module.forward({tensor_img}).toTensor();
            auto percent = output.softmax(1);
            int index = output.argmax(1).item<int>();
            return {items[std::to_string(index)],percent[0][index].item<float>()*100};
        }

        std::vector<std::pair<std::string,float>> predict_topk(const std::string& img_path,unsigned int k)
        {
            if(k <= 0) return {};
            torch::Tensor tensor_img = load_img(img_path);
            torch::Tensor output = module.forward({tensor_img}).toTensor();
            auto percent = output.softmax(1);
            auto topk = std::get<1>(output.topk(k >= output.size(1) ? output.size(1) : k,1,true,true));

            std::vector<std::pair<std::string,float>> res;
            k = k >= output.size(1) ? output.size(1) : k;
            for(int i=0;i<k; i++)
            {
                int index = topk[0][i].item<int>();
                res.emplace_back(
                        items[std::to_string(index)],percent[0][index].item<float>()*100
                        );
            }
            return res;
        }
    };
}

#endif
