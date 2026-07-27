#include <iostream>
#include <vector>

// Moderate complexity: bubble sort (nested loops)
void bubble_sort(std::vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                // Swap elements
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Moderate complexity: binary search (loop with conditions)
int binary_search(const std::vector<int>& arr, int target) {
    int left = 0;
    int right = arr.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == target) {
            return mid;
        }
        if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

int main() {
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    std::cout << "Original array: ";
    for (int num : data) std::cout << num << " ";
    std::cout << std::endl;

    bubble_sort(data);

    std::cout << "Sorted array: ";
    for (int num : data) std::cout << num << " ";
    std::cout << std::endl;

    int target = 22;
    int index = binary_search(data, target);
    if (index != -1) {
        std::cout << "Found " << target << " at index " << index << std::endl;
    } else {
        std::cout << target << " not found" << std::endl;
    }

    return 0;
}
