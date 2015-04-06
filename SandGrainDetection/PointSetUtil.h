#include <iostream>
#include <fstream>
#include <string>

#include "itkMesh.h"
#include "itkPointSet.h"
#include "itkTransformMeshFilter.h"


// Writes PointSet 'points' to file named 'filename'
template <typename TElement, uint32_t TDimension>
int32_t writeToFile(std::string filename,
                    typename itk::PointSet<TElement, TDimension>::Pointer points)
{
    // Don't need to write any points if the size is less than or equal to 0
    if (points->GetNumberOfPoints() <= 0) {
        return -1;
    }

    // Writing binary file
    std::ofstream pointsFile(filename, std::ios::out);
    const uint32_t numPoints = uint32_t(points->GetNumberOfPoints());

    typename itk::PointSet<TElement, TDimension>::PointType p;

    if (pointsFile.is_open()) {
        // Write out metadata of PointSet (number of points, dimension of each point)
        pointsFile << numPoints << std::endl;
        p = points->GetPoint(0);
        pointsFile << TDimension << std::endl;

        // Iterate through pointset, write each individual field to file
        // Each line is a point, fields separated by space
        for (auto i = 0; i < numPoints; ++i) {
            p = points->GetPoint(i);
            for (auto i = 0; i < p.GetPointDimension(); ++i) {
                pointsFile << p[i] << " ";    
            }
            pointsFile << std::endl;
        }

        return 0;
    }

    return -1;
}

// Read PointSet from file named 'filename' and return pointer to it
template <typename TElement, uint32_t TDimension>
typename itk::PointSet<TElement, TDimension>::Pointer
readFromFile(std::string filename)
{
    std::ifstream pointsFile(filename, std::ios::in);
    if (pointsFile.is_open()) {
        // Read in metadata (number of points, dimension of each point)
        uint32_t numPoints = 0;
        pointsFile >> numPoints;
        uint32_t pointDimension = 0;
        pointsFile >> pointDimension;

        // Make sure our dimensionalities match up
        if (pointDimension != TDimension) {
            std::cerr << "[error]: trying to read " << pointDimension << "D PointSet into "
                      << TDimension << "D PointSet" << std::endl;
            exit(1);
        }

        // Read in point data
        typename itk::PointSet<TElement, TDimension>::PointType p;
        auto points = itk::PointSet<TElement, TDimension>::New();
        for (auto i = 0; i < numPoints; ++i) {
            for (auto j = 0; j < pointDimension; ++j) {
                pointsFile >> p[j];
            }
            points->SetPoint(i, p);
        }

        return points;
    } else {
        return nullptr;
    }
}

template <typename TElement, uint32_t TDimension>
typename itk::Mesh<TElement, TDimension>::Pointer
pointSet2Mesh(typename itk::PointSet<TElement, TDimension>::Pointer points)
{
    auto mesh = itk::Mesh<TElement, TDimension>::New();
    for (auto i = 0; i < points->GetNumberOfPoints(); ++i) {
        mesh->SetPoint(i, points->GetPoint(i));
    }
    return mesh;
}

template <typename TElement, uint32_t TDimension>
typename itk::PointSet<TElement, TDimension>::Pointer
mesh2PointSet(typename itk::Mesh<TElement, TDimension>::Pointer points)
{
    auto pointSet = itk::PointSet<TElement, TDimension>::New();
    for (auto i = 0; i < points->GetNumberOfPoints(); ++i) {
        pointSet->SetPoint(i, points->GetPoint(i));
    }
    return pointSet;
}



// Transform PointSet by applying specific Transform
// 'transform' MUST be set up before this (e.g. with your params and such)
template <typename TElement, uint32_t TDimension, typename TTransform>
typename itk::PointSet<TElement, TDimension>::Pointer
applyTransform(typename itk::PointSet<TElement, TDimension>::Pointer points,
               typename TTransform::Pointer transform)
{
    // Need to convert to mesh first to do transformation
    auto mesh = pointSet2Mesh<TElement, TDimension>(points);

    // Create transform
    using TMesh = itk::Mesh<TElement, TDimension>;
    auto transformFilter = itk::TransformMeshFilter<TMesh, TMesh, TTransform>::New();
    transformFilter->SetInput(mesh);
    transformFilter->SetTransform(transform);

    // Run transform
    try {
        transformFilter->Update();
    } catch (itk::ExceptionObject& ex) {
        std::cerr << "[error]: itk::ExceptionObject caught" << std::endl;
        std::cerr << ex << std::endl;
        exit(1);
    }

    // Convert back to PointSet
    auto transformedPoints = mesh2PointSet<TElement, TDimension>(transformFilter->GetOutput());
    return transformedPoints;
}
